cmake_minimum_required(VERSION 3.10)
project(op_prime_number)

set(CMAKE_CXX_STANDARD 17)

add_executable(op_prime_number main.cpp primes.cpp primes.h test.cpp)