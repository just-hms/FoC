# Define your compiler and flags
CC := g++
CFLAGS:=-std=c++2a -w
FLAGS:=-lcrypto -ljsoncpp -lncurses

# Define build directory
BUILD_DIR := build


# If the first argument is "run"... then set everything else as arguments
ifeq (run,$(firstword $(MAKECMDGOALS)))
  # use the rest as arguments for "run"
  RUN_ARGS := $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))
  # ...and turn them into do-nothing targets
  $(eval $(RUN_ARGS):;@:)
endif

# List your source files
SRC := $(shell find src -name '*.cpp' ! -name '*_test.cpp' ! -wholename '*/cmd/*')
CMD_SRC := $(shell find src/cmd -name '*.cpp')
TEST_SRC := /tmp/test.cpp

# Define corresponding object files in the build directory
OBJ := $(SRC:%.cpp=$(BUILD_DIR)/%.o)
CMD_OBJ := $(CMD_SRC:%.cpp=$(BUILD_DIR)/%.o)
TEST_OBJ := $(TEST_SRC:%.cpp=$(BUILD_DIR)/%.o)

# Default target
all: test build run

# Test target
test: $(BUILD_DIR)/test
	@$(BUILD_DIR)/test

# Rule to generate test.cpp and compile it into an object file
$(BUILD_DIR)/test: $(OBJ) $(TEST_OBJ)
	@$(CC) $(CFLAGS) -o $@ $^ $(FLAGS)

$(TEST_OBJ): $(TEST_SRC)
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -c $< -o $@ $(FLAGS)

# Rule to run test.sh and generate test.cpp
$(TEST_SRC):
	@./test.sh > $(TEST_SRC)

# Build cmds target
build: $(CMD_OBJ) $(OBJ)
	@$(foreach cmd, $(CMD_OBJ), $(CC) $(CFLAGS) -o $(BUILD_DIR)/$(notdir $(cmd:.o=)) $(cmd) $(OBJ) $(FLAGS);)

# Run target
run: build
	@$(BUILD_DIR)/$(RUN_ARGS)

# General rule for object files
$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -c $< -o $@ $(FLAGS)

# Clean target to remove generated files
clean:
	@rm -rf $(BUILD_DIR) $(TEST_SRC)

.PHONY: all test build run clean $(TEST_OBJ) $(BUILD_DIR)/test
