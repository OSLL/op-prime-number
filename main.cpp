#include <iostream>                  // cerr
#include <fstream>                   // ifstream, ofstream
#include <string>
#include <tuple>
#include <sstream>
#include <algorithm>                // copy_if()
#include <iterator>                 // istream_iterator<>, ostream_iterator<>
#include <experimental/filesystem>  // canonnical(), path()
#include <cstdlib>                  // exit()
#include <cstring>                  // atoi()
#include <getopt.h>                 // getopt_long()
#include <unistd.h>                 // _SC_NPROCESSORS_ONLN, for()
#include <wait.h>                   // wait()

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

// void worker(const fs::path & in_path_file, const fs::path & output_path_file, const what & task) - процедура,
// которая, обрабатывает файл с числами указанный в параметре in_path_file, согласно режиму указанном в параметре task
// после чего записывает результат в файл, указанный в параметре output_path_file.
// Принимаемые параметры: in_path_file     - путь к файлу, типа path, с которого будут считываться и
//                                           обрабатываться значения
//                        output_path_file - путь к файлу, в который будут записываться результы
//                        task             - режим обработки
void worker(const fs::path & in_path_file, const fs::path & output_path_file, const what & task);

// void create_files_for_proc(const fs::path & in_path_file, const fs::path & dir_out, long num_files) - процедура
// которая разбивавает файл указанный в параметре in_path_file, на 'num_files' частей указанный в параметре num_files,
// после чего эти файлы будут записаны в директорию указанной в параметре dir_out.
// Принимаемые параметры: in_path_file - путь к файлу, который предстоит разбивать
//                        dir_out      - путь к директории куда будут записываться файлы
//                        num_files    - колличество файлов на которые нужно разбить файл указынный в in_path_file
void create_files_for_proc(const fs::path & in_path_file, const fs::path & dir_out, long num_files);

// fs::path create_path_file(const fs::path & dir) - функция, генеррирующая уникальное имя файла для пути,
// указанном в праметре dir и возвращающая путь к файлу с уникальным именем
// Принимаемые параметры: dir - путь к директории
// Возвращаемые праметры: путь к директории с сгенерированным уникальным именем для пути dir
fs::path create_path_file(const fs::path & dir);

int main(int argc, char * argv[]) {

    // Вызываем get_param(), после чего распаковываем результат работы функции.
    const auto [in_path, out_path, task, num_proc] = get_param(argc, argv);

    if (!fs::exists(in_path)) {  // Если у нас имеется не валидный путь к файлу
        std::cerr << "Invalid path to input. Path does not exist." << std::endl;
        std::exit(EXIT_FAILURE); // Выходим с ошибкой
    }
    // если требуется обработка в num_proc процессов
    if (num_proc >= 0) {
        // Создаем путь для временной дирректории куда будут записаны 'разбитые файлы' для num_proc процессов
        fs::path tmp_input_dir = "./tmp_input_files/";

        // Создаем путь для временной дирректории куда будут записаны результаты работы процедуры worker()
        fs::path tmp_output_dir = "./tmp_results/";

        if (!fs::create_directory(tmp_output_dir)) { // сразу же создадим эту директорию
            std::cerr << "Error: cant create temporary directory" << std::endl;
            std::exit(EXIT_FAILURE);
        }
        // Колличество нужных файлов равно колличеству требуемых процессов. Так для параметра 's' значение опционально,
        // то если значение не указано, количество файлов будет равно колличеству ядер центрального процессора
        long num_files = (num_proc == 0 ? sysconf(_SC_NPROCESSORS_ONLN): num_proc);

        // Как только получили нужные параметры, начинаем разбивать файл
        create_files_for_proc(in_path, tmp_input_dir, num_files);

        // После того как получены файлы для процессов, открываем дирректорию с полученными файлами
        // и запускаем обработку каждого файла в новом процессе
        for (const auto & filename: fs::directory_iterator{tmp_input_dir}) {
            pid_t pid = fork();  // порождаем новый процесс
            if (pid < 0) {       // если ошибка, то тут же выходим
                std::cerr << "Error: cant create process" << std::endl;
                std::exit(EXIT_FAILURE);
            }
            else if (pid > 0)    // Если мы в главном(родительском) процессе
                continue;        // продолжаем порождать процессы
            else {               // Если мы в дочернем процессе, то
                worker(filename, create_path_file(tmp_output_dir), task);  // Запускаем обработку файла
                std::exit(EXIT_SUCCESS);              // Как только обработка завершена, завержаем процесс
            }
        }
        // Подождем завершения всех порожденных процессов в родителе
        std::vector<int> statuses(static_cast<unsigned long>(num_files));
        for (int & status : statuses)
            wait(&status);

        // После того как процессы отработают, начинаем собирать значения в результирующий файл указанный
        // в опции '[-o | --output]' сохраненный в переменной out_path
        std::ofstream out_file{out_path.string(), std::ios_base::out};
        std::string line;
        for (const auto & filename: fs::directory_iterator{tmp_output_dir}) {
            std::ifstream in_file{filename.path().string(), std::ios_base::in};
            while (std::getline(in_file, line))
                out_file << line << std::endl;
            in_file.close();
        }
        out_file.close();
        fs::remove_all(tmp_input_dir);  // Удаляем временную дирректорию
        fs::remove_all(tmp_output_dir); // Удаляем временную дирректорию
    }
    // Ичаче запускаем обработку в однопроцессном режиме
    else {
        worker(in_path, out_path, task);
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
        for (numeric_t num = left; num <= right; ++num) {
            if (Primes::is_prime(std::abs(num)))
                out << num << " ";
        }
    }
    else {
        for (numeric_t num = left; num <= right; ++num) {
            out << "{ " << num << ": ";
            for (const auto & divider: Primes::factorization(num))
                out << divider << " ";
            out << "}";
        }
    }
    out << "]" << std::endl;
}

void worker(const fs::path & in_path_file, const fs::path & output_path_file, const what & task) {
    std::ifstream in_file;                                                    // <--- файл для чтения
    in_file.exceptions(std::ios_base::badbit | std::ios_base::failbit);       // <--- установим флаги исключений
    std::ofstream out_file;                                                   // <--- файл для записи
    out_file.exceptions(std::ios_base::badbit | std::ios_base::failbit);      // <--- установим флаги исключений
    try {
        std::string line;
        in_file.open(fs::canonical(in_path_file).string(), std::ios_base::in);
        out_file.open(output_path_file.string(), std::ios_base::out);
        if (task == what::check) {                               // <--- Если выбран режим проверки чисел на простоту
            while (in_file >> line) {
                if (line.find(':') != std::string::npos)         // <--- Если У нас встретился диапазон
                    process_range(line, out_file, what::check);  // <--- Обрабатываем диапазон
                else
                if (Primes::is_prime(std::abs(std::stoll(line))))
                    out_file << line << std::endl;
            }
        }
        else                                                     // <--- Иначе выбран режим факторизации
            while (in_file >> line) {
                if (line.find(':') != std::string::npos)         // <--- Если У нас встретился диапазон
                    process_range(line, out_file, what::factor); // <--- Обрабатываем диапазон
                else {
                    out_file << line << ": ";
                    for (const auto &divider: Primes::factorization(std::stoll(line)))
                        out_file << divider << " ";
                    out_file << std::endl;
                }
            }
        in_file.close();
        out_file.close();
    }
    catch (std::ios_base::failure & e) {
        if (!in_file.eof()) {
            if (errno)                                      // Если это системная ошибка
                std::cerr << strerror(errno) << std::endl;  // Выводим значение errno
            else
                std::cerr << e.what() << std::endl;         // Иначе это ошибка чтения из потока
            std::exit(EXIT_FAILURE);
        }
    }
}

void create_files_for_proc(const fs::path & in_path_file, const fs::path & dir_out, long num_files) {
    std::ifstream in_file{fs::canonical(in_path_file).string(), std::ios_base::in};
    if (!in_file) {
        std::cerr << "Error: can not open file for reading." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    auto stream_size = std::distance(std::istream_iterator<std::string>{in_file}, std::istream_iterator<std::string>{});
    in_file.clear();
    in_file.seekg(0, std::ios_base::beg);

    if (fs::create_directory(dir_out)) {
        std::string line;
        auto block_size = stream_size / num_files;
        for (long fileno = 0; fileno < num_files - 1; ++fileno) {
            auto path = create_path_file(dir_out);
            std::ofstream out{path.string(), std::ios_base::out};
            for (long i = 0; i < block_size; ++i) {
                in_file >> line;
                out << line << std::endl;
            }
            out.close();
        }
        auto path = create_path_file(dir_out);
        std::ofstream out{path.string(), std::ios_base::out};
        while (in_file >> line)
            out << line << std::endl;
        in_file.close();
    }
    else {
        std::cerr << "It is impossible to create a temporary directory." << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

fs::path create_path_file(const fs::path & dir) {
    std::unique_ptr<char> tmp_filename(strdup(std::string(dir.string() + "XXXXXX").c_str()));
    mkstemp(tmp_filename.get());
    return fs::path{tmp_filename.get()};
}