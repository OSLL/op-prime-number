// primes.h --- класс реализующий операции с числами. Предоставляющий такие основные
//              основные операции как проверка чисел на простоту и факторизация числа

#ifndef OP_PRIME_NUMBER_PRIMES_H
#define OP_PRIME_NUMBER_PRIMES_H

#include <vector>
#include <set>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <algorithm>
#include <memory>
#include <experimental/filesystem>
#include <fstream>
#include <iterator>
#include <cassert>
#include <unistd.h>
#include <wait.h>
#include <string.h>

using numeric_t = long long;


class Prime {
public:
    static bool is_prime(numeric_t num)             noexcept;
    static bool is_prime(numeric_t num, long nproc) noexcept;

    static std::set<numeric_t> factorization(numeric_t num);
};

class NumericIterator {
public:
    explicit NumericIterator(numeric_t pos = 0);
    numeric_t operator * () const;

    NumericIterator & operator ++ ();
    const NumericIterator operator ++ (int);
    NumericIterator & operator += (numeric_t n);
    NumericIterator & operator -= (numeric_t n);
    NumericIterator operator + (numeric_t n) const;
    NumericIterator operator - (numeric_t n) const;
    numeric_t operator - (const NumericIterator & other) const;

    bool operator != (const NumericIterator & other) const;
    bool operator == (const NumericIterator & other) const;
    bool operator <  (const NumericIterator & other) const;

private:
    numeric_t num_;
};

namespace std {
    template <>
    struct iterator_traits<NumericIterator> {
        using iterator_category = std::random_access_iterator_tag;
        using value_type = numeric_t;
        using difference_type = std::ptrdiff_t;
    };
}

class NumericRange {
public:
    NumericRange(numeric_t from, numeric_t to);
    NumericIterator begin() const;
    NumericIterator end() const;
private:
    numeric_t from_;
    numeric_t to_;
};

#endif //OP_PRIME_NUMBER_PRIMES_H
