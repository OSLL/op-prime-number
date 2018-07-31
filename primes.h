// primes.h --- класс реализующий операции с числами. Предоставляющий такие основные
//              основные операции как проверка чисел на простоту и факторизация числа

#ifndef OP_PRIME_NUMBER_PRIMES_H
#define OP_PRIME_NUMBER_PRIMES_H

#include <vector>
#include <utility>
#include <cstdlib>
#include <ctime>

using numeric_t = unsigned long long;

class Primes {
public:
    // Проверка числа на простоту
    //
    static bool is_prime(numeric_t num);

    // Факторизация числа. Возвращает вектор пар разложений числа на простые множители.
    // Пара содержит простотое число в поле first и его степень в поле second
    //
    static std::vector<std::pair<numeric_t , numeric_t>> factorization(numeric_t num);

    // Наибольший общий делитель
    //
    static numeric_t gcd(numeric_t a, numeric_t b);

    // Быстрое умножение по модулю
    //
    static numeric_t mul(numeric_t a, numeric_t b, numeric_t m);

    // Быстрое возведение в степень по модулю
    //
    static numeric_t pow(numeric_t a, numeric_t b, numeric_t m);

    // Решето Эратосфена
    //
    static void fill_sieve(std::vector<bool> & fill);
};


#endif //OP_PRIME_NUMBER_PRIMES_H
