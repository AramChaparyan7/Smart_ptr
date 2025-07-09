CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread
TARGET = run_tests
SRC = main.cpp
LIBS = -lgtest -lgtest_main -pthread

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LIBS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
