CXX      = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -g

SRC_DIR  = src
BIN_DIR  = bin
TARGET   = $(BIN_DIR)/compiler

# All .cpp files in src/ (including subdirectories)
SRCS = $(shell find $(SRC_DIR) -name '*.cpp')
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(BIN_DIR)/%.o,$(SRCS))

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BIN_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

run: $(TARGET)
	./$(TARGET) $(filter-out $@,$(MAKECMDGOALS))

clean:
	find $(BIN_DIR) -name '*.o' -delete 2>/dev/null; rm -f $(TARGET)

%:
	@:
