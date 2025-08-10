CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -O2 -fopenmp

brute-cpu: *.cc *.h
	$(CXX) $(CXXFLAGS) -o $@ brute-cpu.cc keccak.cc sha3_cpu.cc

format:
	clang-format -i *.cc *.h
