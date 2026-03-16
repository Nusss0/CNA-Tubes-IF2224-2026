CXX      = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -g

SRC_DIR  = src
BIN_DIR  = bin
TARGET   = $(BIN_DIR)/compiler

# All .cpp files in src/
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(BIN_DIR)/%.o,$(SRCS))

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BIN_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

run: $(TARGET)
	./$(TARGET) test/M1/input1.txt

clean:
	rm -f $(BIN_DIR)/*.o $(TARGET)
