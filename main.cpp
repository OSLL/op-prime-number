#include <iostream>                  // cerr
#include <fstream>                   // ifstream, ofstream
#include <string>
#include <tuple>
#include <algorithm>                // copy_if()
#include <iterator>                 // istream_iterator<>, ostream_iterator<>
#include <experimental/filesystem>  // canonnical(), path()
#include <cstdlib>                  // exit()
#include <cstring>                  // atoi()
#include <getopt.h>                 // getopt_long()

#include "primes.h"


namespace fs = std::experimental::filesystem;  // Для удобства объявим псевдоним fs для filesystem

enum class what {check, factor, empty};        // check -- выполнить проверку на простоту, factor -- разлодить число на
                                               // простые множители, empty -- отсутсвие параметра check либо factor

void usage();                                  // <--- справка по использованию программы
void help();                                   // <--- информация по опциям и параметрам программы


// std::tuple<fs::path, fs::path, what, int> get_param(int argc, char * argv[]) - функция обработки параметров
// командной строки.
// Принимаемые параметры: arc - число параметров, argv - массив указателей на строки
// Возвращаемое значение: возращает кортеж, содержащий такие значения как:
//                        path (первый в списке) --- путь к к файлу в котором содердится список чисел для обработки,
//                        path (второй в списке) --- путь к выходному файлу, в который будут записываться
//                                                   результы работы программы
//                        what --- содержит одно из трех перечислений, указывающих режим
//                                 исполнения программы. см. enum what {check, factor, empty}
std::tuple<fs::path, fs::path, what, int> get_param(int argc, char * argv[]);


int main(int argc, char * argv[]) {

    // Вызываем get_param(), после чего распаковываем результат работы функции.
    // В данной программе переменная num_proc пока не используется так как она зарезервирована
    // для параллельной обработки списков чисел.
    const auto [in_path, out_path, task, num_proc] = get_param(argc, argv);

    // Чтобы не вылезало предупреждение
    (num_proc == 0);

    if (!fs::exists(in_path)) {  // Если у нас имеется не валидный путь к файлу
        std::cerr << "Invalid path to input. Path does not exist." << std::endl;
        std::exit(EXIT_FAILURE); // Выходим с ошибкой
    }

    std::ifstream in_file;                                                    // <--- файл для чтения
    in_file.exceptions(std::ios_base::badbit | std::ios_base::failbit);       // <--- установим флаги исключений
    std::ofstream out_file;                                                   // <--- файл для записи
    out_file.exceptions(std::ios_base::badbit | std::ios_base::failbit);      // <--- установим флаги исключений
    try {
        in_file.open(fs::canonical(in_path).string(), std::ios_base::in);
        out_file.open(out_path.string(), std::ios_base::out);
        if (task == what::check) {                               // <--- Если выбран режим проверки чисел на простоту
            std::copy_if(std::istream_iterator<numeric_t>{in_file}, std::istream_iterator<numeric_t>{},
                    std::ostream_iterator<numeric_t>{out_file, "\t"},
                            [] (numeric_t num) { return Primes::is_prime(num); });
        }
        else {                                                   // <--- Иначе выбран режим факторизации
            numeric_t num;
            while (in_file >> num) {
                for (const auto & pair: Primes::factorization(num))
                    out_file <<"(" << pair.first << "^" << pair.second << ") * ";
                out_file << "1" << std::endl;
            }
        }
    }
    catch (std::ios_base::failure & e) {
        if (!in_file.eof()) {
            if (errno)                                      // Если это системная ошибка
                std::cerr << strerror(errno) << std::endl;  // Выводим значение errno
            else
                std::cerr << e.what() << std::endl;         // Иначе это ошибка чтения из потока
        }
    }
    return 0;
}

void usage() {
    std::cout << "usage: program_name [-p | --path [path to file]: required]" << std::endl
              << "                    [-o | --output [path to file]: required]" << std::endl
              << "                    [-s | --scale [value: optional]: optional]" << std::endl
              << "                    [-с | --check: required]" << std::endl
              << "                    [-f | --factor: required]" << std::endl
              << "type program_name [-h | --help]" << std::endl;
}

void help() {
    usage();
    std::cout << "\n[-p | --path [path to file]] - The path to a file with lists of numbers\n"
                 "to process. Option is required\n"
              << "[-o | --output [path to file]] - Output file. In which the results will be recorded.\n"
                 "Option is required\n"
                 "[-с | --check]  - Checking lists of numbers for simplicity\n"
                 "[-f | --factor] - Decomposition of numbers into prime divisors\n"
              << "[-s | --scale]  - This option tells the program to make the list processing parallel to the number.\n"
                 "The value of the option indicates how many processes to split the processing of the list of numbers.\n"
                 "If the value is not specified, "
                 "then the selection of the number of processes will occur automatically." << std::endl;
}

std::tuple<fs::path, fs::path, what, int> get_param(int argc, char * argv[]) {
    if (argc < 2) {
        usage();
        std::exit(EXIT_FAILURE);
    }
    const char * short_options = "hp:o:s::cf";
    const option long_options[] = {
            {"help", no_argument, nullptr, 'h'},         {"path", required_argument, nullptr, 'p'},
            {"output", required_argument, nullptr, 'o'}, {"scale", optional_argument, nullptr, 's'},
            {"check", no_argument, nullptr, 'c'},        {"factor", no_argument, nullptr, 'f'},
            {nullptr, 0, nullptr, 0}
    };
    int res{};
    int option_index = -1;

    fs::path input{}, output{};
    int num_proc{};
    what task = what::empty;
    while ((res = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1) {
        switch (res) {
            case 'h': help();          break;
            case 'p': input  = optarg; break;
            case 'o': output = optarg; break;
            case 'c': task   = (task == what::empty ? what::check: task);  break;
            case 'f': task   = (task == what::empty ? what::factor: task); break;
            case 's': num_proc = (optarg ? atoi(optarg): 0);
                break;
            case '?':
            default: usage(); break;
        }
        option_index = -1;
    }
    if (input.empty() || output.empty() || task == what::empty) {
        std::cout << "not enough options needed" << std::endl;
        usage();
        std::exit(EXIT_FAILURE);
    }
    return {input, output, task, num_proc};
}
