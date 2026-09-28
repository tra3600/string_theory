CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra

string_theory: string_theory.cpp src/*.hpp
	$(CXX) $(CXXFLAGS) string_theory.cpp -o string_theory

test: string_theory
	./string_theory --test

figures: string_theory
	./string_theory --rapport figures

clean:
	rm -f string_theory

.PHONY: test figures clean
