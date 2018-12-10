#include "primes.h"

namespace fs = std::experimental::filesystem;

// static void wait_all(std::vector<pid_t> & childs) - процедура, которая ждет завершения
// всех процессов указанных в параметре childs;
// Принимаемые параметры: childs --- вектор pid процессов, завершения которых следует ждать;
// Возвращаемы параметры: нет.
static void wait_all(std::vector<pid_t> & childs);

// static void term_all(std::vector<pid_t> & childs) - процедура, которая завершает все
// процессы в указанном параметре childs;
// Принимаемые параметры: childs --- вектор pid процессов, которые следует завершить;
// Возвращаемые параметры: нет.
static void term_all(std::vector<pid_t> & childs);

// void handler(int sig) - простой обработчик сигнала
void handler(int sig);

// static numeric_t gcd (numeric_t a, numeric_t b) - функция возращающая НОД числа 'b' и 'а';
// Принимаемые параметры: числа 'a' и 'b';
// Возвращаемы параметрые: НОД чисел 'a' и 'b'.
static numeric_t gcd (numeric_t a, numeric_t b);

// static numeric_t custom_mul(numeric_t param, numeric_t mod) - функция возвращающая результат
// умножения числа param и param + 1 по модулю mod;
static numeric_t custom_mul(numeric_t param, numeric_t mod);

// static std::set<numeric_t> simple_factor(numeric_t num) - функция реализующая происк
// простых делителей числа num простым пробным делением.
// Принимаемые параметры : num --- число которое следует факторизовать;
// Возвращаемые параметры: множество простых делителей числа num типа std::set<numeric_t>
static std::set<numeric_t> simple_factor(numeric_t num);

// static numeric_t pollard_rho(numeric_t number) - алгоритм факторизации числа Полларда - ро
// Принимаемые параметры : num --- число, первый делитель которого следует найти;
// Возвращаемые параметры: первый делитель числа num.
static numeric_t pollard_rho(numeric_t num);


// ----------------------------------- Реализация методов класса Prime -------------------------------------------------

// bool Primes::is_prime(numeric_t num) - метод реализует алгоритм
// проверки числа на простоту.
// Принимаемые параметры:  num типа numeric_t(псевдоним числового типа).
// Возвращаемые параметры: true, если num простое число, false - в противном случае
bool Prime::is_prime(numeric_t num) noexcept {
    numeric_t mod = std::abs(num);
    if ((mod != 2 && num % 2 == 0) || mod == 1 || mod == 0)
        return false;
    NumericRange range(3, static_cast<numeric_t>(std::sqrt(mod)) + 1);
    for (auto it = range.begin(); it < range.end(); it += 2)
        if (num % (*it) == 0)
            return false;
    return true;
}


static volatile sig_atomic_t prime;


// bool Primes::is_prime(numeric_t num, long nproc) - метод реализует алгоритм
// проверки числа на простоту.
// Принимаемые параметры:  num   --- типа numeric_t(псевдоним числового типа).
//                         nproc --- колличество процессов
// Возвращаемые параметры: true, если num простое число, false - в противном случае
bool Prime::is_prime(numeric_t num, long nproc) noexcept {
    if (nproc <= 0 || nproc == 1 || std::abs(num) < 1000000)
        return Prime::is_prime(num);

    numeric_t mod = std::abs(num);
    if (mod % 2 == 0 || mod % 3 == 0 || mod % 5 == 0 || mod % 7 == 0 || mod % 11 == 0 ||
        mod % 13 == 0 || mod % 17 == 0 || mod % 19 == 0 || mod % 23 == 0 || mod % 29 == 0)
        return false;

    prime = true;
    NumericRange range(3, static_cast<numeric_t>(std::sqrt(mod)) + 1);
    if (std::distance(range.begin(), range.end()) < nproc)
        return Prime::is_prime(mod);

    auto block_size = std::distance(range.begin(), range.end()) / nproc;
    std::vector<pid_t> childs;

    signal(SIGUSR1, handler);
    auto block_start = range.begin();


    std::cout << "\n-----------------------------------------------------------------------------------------------\n";
    std::cout << "number is  " << num << std::endl;

    for (long i = 0; i < nproc - 1 && prime; ++i) {

        auto block_end = block_start;
        std::advance(block_end, block_size);

        childs.push_back(fork());
        if (childs.back() < 0) {
            std::cerr << "is_prime: Can't create process." << std::endl;
            std::exit(EXIT_FAILURE);
        }
        else if (childs.back() > 0) {
            block_start = block_end;
            continue;
        }
        else {
            std::cout << "\tcreated process: " << getpid()
                      << " process interval: [" << *block_start << ":" << *block_end << "]";
            for (auto it = (*block_start % 2 == 0 ? ++block_start: block_start); it < block_end; it += 2)
                if (mod % (*it) == 0) {
                    std::cout << " <-- there is a divider!" << std::endl;
                    kill(getppid(), SIGUSR1);
                    std::exit(EXIT_SUCCESS);
                }
            std::cout << " <-- divider not found" << std::endl;
            std::exit(EXIT_SUCCESS);
        }
    }
    if (!prime) {
        term_all(childs);
        return false;
    }
    wait_all(childs);
    std::cout << "\tcreated process: " << getpid()
              << " process interval: [" << *block_start << ":" << *range.end() << "]";
    if (prime) {
        for (auto it = (*block_start % 2 == 0 ? ++block_start : block_start); it < range.end(); it += 2)
            if (mod % (*it) == 0)
                return false;
        std::cout << " <-- divider not found" << std::endl;
        std::cout << "\ntotal: " << childs.size() + 1 << std::endl;
    }
    else {
        std::cout << " <-- not checked";
        std::cout << "\ntotal: " << childs.size() << std::endl;
    }
    std::cout << "-----------------------------------------------------------------------------------------------\n";
    return prime != 0;
}

// std::vector<numeric_t> Primes::factorization(numeric_t num) -
// метод, возвращающий множество простых делителей числа типа std::set<numeric_t>.
std::set<numeric_t> Prime::factorization(numeric_t num) {
    numeric_t mod = std::abs(num);
    if (mod == 1 || Prime::is_prime(mod))
        return {num};
    if (mod == 0) return {};
    if (mod < 1000000)
        return simple_factor(num);

    std::set<numeric_t> result;
    while (mod != 1) {
        numeric_t divider = pollard_rho(mod);
        mod /= divider;
        result.insert(divider);
    }
    if (num < 0) {
        auto key = *result.begin();
        result.erase(key);
        result.insert(-key);
    }
    return result;
}

// ---------------------------------------------------------------------------------------------------------------------

// -------------- Реализация класса числового итератора NumericIterator и Числового диапазона NumericRange -------------

NumericIterator::NumericIterator(numeric_t pos): num_{pos} {}

numeric_t NumericIterator::operator*() const { return num_; }

NumericIterator& NumericIterator::operator++() {
    ++num_;
    return *this;
}

const NumericIterator NumericIterator::operator++(int) {
    NumericIterator ret(num_);
    ++(*this);
    return ret;
}

NumericIterator& NumericIterator::operator+=(numeric_t n) {
    num_ += n;
    return *this;
}

NumericIterator& NumericIterator::operator-=(numeric_t n) {
    num_ -= n;
    return *this;
}

NumericIterator NumericIterator::operator+(numeric_t n) const {
    return NumericIterator(num_ + n);
}

NumericIterator NumericIterator::operator-(numeric_t n) const {
    return NumericIterator(num_ - n);
}

numeric_t NumericIterator::operator-(const NumericIterator &other) const {
    return num_ - other.num_;
}

bool NumericIterator::operator != (const NumericIterator & other) const {
    return num_ != other.num_;
}

bool NumericIterator::operator == (const NumericIterator &other) const {
    return !(*this != other);
}

bool NumericIterator::operator<(const NumericIterator &other) const {
    return num_ < other.num_;
}

NumericRange::NumericRange(numeric_t from, numeric_t to): from_{from}, to_{to} {}

NumericIterator NumericRange::begin() const { return NumericIterator{from_}; }

NumericIterator NumericRange::end() const { return NumericIterator{to_}; }

// --------------------------------------------------------------------------------------------------------------------



// --------------------- Реализация статических вспомогательных функций -----------------------------------------------

static void wait_all(std::vector<pid_t> & childs) {
    for (pid_t pid: childs)
        while (waitpid(pid, nullptr, 0) > 0);
}

static void term_all(std::vector<pid_t> & childs) {
    for (pid_t pid: childs) {
        std::cout << "kill pid: " << pid << std::endl;
        kill(pid, SIGTERM);
    }
}


void handler(int sig) {
    sigset_t set;
    if (sigemptyset(&set))                      return;
    if (sigaddset(&set, SIGUSR1))               return;
    if (sigprocmask(SIG_BLOCK, &set, nullptr))  return;

    prime = false;

    if (sigprocmask(SIG_UNBLOCK, &set, nullptr)) return;
}


static std::set<numeric_t> simple_factor(numeric_t num) {
    numeric_t mod = std::abs(num);
    if (mod == 1 || Prime::is_prime(mod))
        return {num};
    if (mod == 0) return {};
    std::set<numeric_t> result;
    NumericRange range(2, mod / 2 + 1);
    std::copy_if(range.begin(), range.end(), std::inserter(result, result.begin()),
                 [&] (numeric_t n) { return mod % n == 0 && Prime::is_prime(n); });
    if (num < 0) {
        auto key = *result.begin();
        result.erase(key);
        result.insert(-key);
    }
    return result;
}

static numeric_t gcd (numeric_t a, numeric_t b) {
    while (b) {
        a %= b;
        std::swap(a, b);
    }
    return a;
}

static numeric_t custom_mul(numeric_t param, numeric_t mod){
    return ((param % mod * (param + 1) % mod) % mod);
}

static numeric_t pollard_rho(numeric_t num){
    numeric_t a = 2, b = 2, ret{};
    while (true)
    {
        a = custom_mul(a, num);
        b = custom_mul(custom_mul(b, num), num);
        ret = gcd(std::abs(b - a), num);
        if (ret > 1)
            break;
    }
    auto sqr = static_cast<numeric_t>(std::sqrt(ret));
    return ret == numeric_t(sqr * sqr) ? sqr: ret;
}

// ---------------------------------------------------------------------------------------------------------------------