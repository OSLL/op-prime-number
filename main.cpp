#include <iostream>                  // cerr
#include <fstream>                   // ifstream, ofstream
#include <string>
#include <tuple>
#include <sstream>
#include <algorithm>                // copy_if()
#include <experimental/filesystem>  // canonnical(), path()
#include <cstdlib>                  // exit()
#include <cstring>                  // atoi()
#include <getopt.h>                 // getopt_long()
#include <unistd.h>                 // _SC_NPROCESSORS_ONLN, for()

#include "primes.h"


namespace fs = std::experimental::filesystem;  // Для удобства объявим псевдоним fs для filesystem

enum class what {check, factor, empty};        // check -- выполнить проверку на простоту, factor -- разложить число на
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
std::tuple<fs::path, fs::path, what, long> get_param(int argc, char * argv[]);

// void process_range(const std::string & range, std::ostream & out, const what & task) - процедура, которая обрабатывает
// число заданное диапазоном в строковом параметре range, после чего обрабатывает этот диапазон и записывает
// его в поток out, согласно режиму обработки task.
// Принимаемые праметры: range - диапазон, представленный строкой в форме "left:right",
//                               где left --- левая граница диапазона, right - правая граница диапазона включительно
//                       out   - выходной поток для записи
//                       task  - режим обработки диапазона
void process_range(const std::string & range, std::ostream & out, const what & task);

// void process_range(const std::string & range, std::ostream & out, const what & task, long nproc) - процедура,
// которая обрабатывает число заданное диапазоном в строковом параметре range, после чего обрабатывает этот диапазон
// и записывает его в поток out, согласно режиму обработки task в nproc процессов.
// Принимаемые праметры: range - диапазон, представленный строкой в форме "left:right",
//                               где left --- левая граница диапазона, right - правая граница диапазона включительно
//                       out   - выходной поток для записи
//                       task  - режим обработки диапазона
//                       nproc - колличество процессов
void process_range(const std::string & range, std::ostream & out, const what & task, long nproc);

void check_for_primes(std::istream & in_file, std::ostream & out_file);
void check_for_primes(std::istream & in_file, std::ostream & out_file, long nproc);

void perform_factorization(std::istream & in_file, std::ostream & out_file);



int main(int argc, char * argv[]) {

    // Вызываем get_param(), после чего распаковываем результат работы функции.
    const auto [in_path, out_path, task, num_proc] = get_param(argc, argv);

    if (!fs::exists(in_path)) {  // Если у нас имеется не валидный путь к файлу
        std::cerr << "Invalid path to input. Path does not exist." << std::endl;
        std::exit(EXIT_FAILURE); // Выходим с ошибкой
    }
    if (fs::is_empty(in_path))
        return 0;

    std::ifstream in_file;
    std::ofstream out_file;

    in_file.exceptions(std::ios_base::badbit | std::ios_base::failbit);
    out_file.exceptions(std::ios_base::badbit | std::ios_base::failbit);

    try {
        std::string line;
        in_file.open(fs::canonical(in_path).string(), std::ios_base::in);
        out_file.open(out_path.string(), std::ios_base::out);

        if (num_proc == -1) {
            if (task == what::check)
                check_for_primes(in_file, out_file);
            else if (task == what::factor)
                perform_factorization(in_file, out_file);
        }
        else if (num_proc >= 0) {
            auto nproc = (num_proc == 0 ? sysconf(_SC_NPROCESSORS_ONLN): num_proc);
            if (task == what::check)
                check_for_primes(in_file, out_file, nproc);
            else if (task == what::factor)
                perform_factorization(in_file, out_file);
        }
    }
    catch (std::ios_base::failure & e) {
        if (!in_file.eof()) {
            if (errno)                                      // Если это системная ошибка
                std::cerr << strerror(errno) << std::endl;  // Выводим значение errno
            else
                std::cerr << e.what() << std::endl;         // Иначе это ошибка чтения из потока
            std::exit(EXIT_FAILURE);
        }
        if (out_file.tellp() == 0)
            out_file << " ----------------- No records --------------------";
    }
    std::cout << std::endl;
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

std::tuple<fs::path, fs::path, what, long> get_param(int argc, char * argv[]) {
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
    int num_proc = -1;
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
        std::cout << "there are not enough options or options are incorrect" << std::endl;
        usage();
        std::exit(EXIT_FAILURE);
    }
    return {input, output, task, num_proc};
}

void process_range(const std::string & range, std::ostream & out, const what & task) {
    std::stringstream ss{range};
    numeric_t left, right;
    char delim;
    ss >> left >> delim >> right;
    out << left << ":" << right << " ---> [ ";
    if (task == what::check) {
        for (numeric_t num = left; num <= right; ++num)
                if (Prime::is_prime(num))
                    out << num << " ";
    }
    else {
        for (numeric_t num = left; num <= right; ++num) {
            out << "{ " << num << ": ";
            if (num != 0)
                for (const auto & divider: Prime::factorization(num))
                    out << divider << " ";
            else
                out << "any";
            out << "}";
        }
    }
    out << "]" << std::endl;
}

void process_range(const std::string & range, std::ostream & out, const what & task, long nproc){
    std::stringstream ss{range};
    numeric_t left, right;
    char delim;
    ss >> left >> delim >> right;
    out << left << ":" << right << " ---> [ ";
    if (task == what::check) {
        for (numeric_t num = left; num <= right; ++num)
            if (Prime::is_prime(num, nproc))
                out << num << " ";
    }
    else {
        for (numeric_t num = left; num <= right; ++num) {
            out << "{ " << num << ": ";
            if (num != 0)
                for (const auto & divider: Prime::factorization(num))
                    out << divider << " ";
            else
                out << "any";
            out << "}";
        }
    }
    out << "]" << std::endl;
}

void check_for_primes(std::istream & in_file, std::ostream & out_file) {
    std::string line;
    while (in_file >> line) {
        if (line.find(':') != std::string::npos) {
            process_range(line, out_file, what::check);
        }
        else {
            try {
                numeric_t num = std::stoll(line);
                if (Prime::is_prime(num))
                    out_file << line << std::endl;
            }
            catch (std::logic_error & e) {
                std::cerr << std::endl << "Number: " << '\'' << line << "' Wrong format or type overflow. "
                                       << "An integer value is required. Number skipped." << std::endl;
                continue;
            }
        }
    }
}

void check_for_primes(std::istream & in_file, std::ostream & out_file, long nproc) {
    std::string line;
    while (in_file >> line) {
        if (line.find(':') != std::string::npos)
            process_range(line, out_file, what::check, nproc);
        else {
            try {
                numeric_t num = std::stoll(line);
                if (Prime::is_prime(num, nproc))
                    out_file << line << std::endl;
            }
            catch (std::logic_error & e) {
                std::cerr << std::endl << "Number: " << '\'' << line << "' Wrong format or type overflow. "
                          << "An integer value is required. Number skipped." << std::endl;
                continue;
            }
        }
    }
}

void perform_factorization(std::istream & in_file, std::ostream & out_file) {
    std::string line;
    while (in_file >> line) {
        if (line.find(':') != std::string::npos)
            process_range(line, out_file, what::factor);
        else {
            try {
                numeric_t num = std::stoll(line);
                out_file << line << ": ";
                for (const auto &divider: Prime::factorization(num))
                    out_file << divider << " ";
                out_file << std::endl;
            }
            catch (std::logic_error & e) {
                std::cerr << std::endl << "Number: " << '\'' << line << "' Wrong format or type overflow. "
                          << "An integer value is required. Number skipped." << std::endl;
                continue;
            }
        }
    }
}