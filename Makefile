TARGET    = op-prime-number
COMPILIER = g++
FLAGS     = -Wall -pedantic -std=c++17 -lstdc++fs -o2

$(TARGET): primes.cpp main.cpp
	$(COMPILIER) primes.cpp main.cpp $(FLAGS) -o $(TARGET)

clean:
	rm -f $(TARGET)
