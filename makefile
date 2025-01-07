# Makefile

CXX = g++
CXXFLAGS = -std=c++17 -Wall -g -flto
LDFLAGS = -static -lyaml-cpp -static-libgcc -static-libstdc++ -flto

# The default target to build when typing 'make' with no arguments:
all: hanabi

# Build the 'main' executable
hanabi: src/parse.cpp
	$(CXX) $(CXXFLAGS) src/parse.cpp -o hanabi $(LDFLAGS)

# Clean up generated files
clean:
	rm -f hanabi