CXX = g++
CXXFLAGS = -std=c++17 -Wall -O3

SRC = src/main.cpp \
      src/parser.cpp \
      src/solver.cpp

TARGET = SATzillaX

all:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)
