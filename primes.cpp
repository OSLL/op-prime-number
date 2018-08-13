#include "primes.h"

// bool Primes::is_prime(numeric_t num) - метод реализует вероятностный алгоритм
// проверки числа на простоту используя малую теорему Ферма.
// Принимаемые параметры:  num типа numeric_t(псевдоним числового типа).
// Возвращаемые параметры: true, если num простое число, false - в противном случае
bool Primes::is_prime(numeric_t num) {
    if (num == 2)
        return true;
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    for (int i = 0; i < 100; ++i) {
        numeric_t a = (std::rand() % (num - 2)) + 2;
        if (gcd(a, num) != 1)
            return false;
        if (pow(a, num - 1, num) != 1)
            return false;
    }
    return true;
}

// std::vector<numeric_t> Primes::factorization(numeric_t num) -
// метод, возвращающий вектор простых делителей числа.
std::vector<numeric_t> Primes::factorization(numeric_t num) {

    if (num == 0)
        return {};

    if (is_prime(std::abs(num)))
        return {num};

    std::vector<numeric_t> result;
    std::vector<bool> primes(std::abs(num) / 2 + 1, true); // чтобы не передать в конструктор отрицительное
    fill_sieve(primes);
    for (std::size_t i = 2; i < primes.size(); ++i)
        if (primes[i] && !(num % static_cast<numeric_t>(i)))
            result.push_back(static_cast<numeric_t>(i));
    if (num < 0)
        result[0] = -result[0];
    return result;
}

numeric_t Primes::gcd(numeric_t a, numeric_t b) {
    if (b == 0)
        return a;
    return gcd(b, a % b);
}

numeric_t Primes::mul(numeric_t a, numeric_t b, numeric_t m) {
    if (b == 1)
        return a;
    if (b % 2 == 0) {
        numeric_t t = mul(a, b / 2, m);
        return (2 * t) % m;
    }
    return (mul(a, b - 1, m) + a) % m;
}

numeric_t Primes::pow(numeric_t a, numeric_t b, numeric_t m) {
    if (b == 0)
        return 1;
    if (b % 2 == 0) {
        numeric_t t = pow(a, b / 2, m);
        return mul(t, t, m) % m;
    }
    return (mul(pow(a, b - 1, m), a, m)) % m;
}

void Primes::fill_sieve(std::vector<bool> & fill) {
    fill[0] = fill[1] = false;
    for (std::size_t i = 2; (i * i < fill.size()); ++i)
        if (fill[i])
            for (std::size_t j = i + i; j < fill.size(); j += i)
                fill[j] = false;
}