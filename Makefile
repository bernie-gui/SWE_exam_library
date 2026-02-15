# Libreria Ingegneria Software
# Anno Accademico 2025-2026 - Università di Roma La Sapienza

# Project settings
TARGET      := main
SRC_DIR     := src
INC_DIR     := include
BUILD_DIR   := build
BINS_DIR    := example_bins

# Tools
CXX         := g++
CXXSTD      := -std=c++17

# Build type (can be overridden: make BUILD=use)
BUILD       ?= use

# Common flags
WARNINGS    := -Wall -Wextra -Wpedantic
INCLUDES    := -I$(INC_DIR)

ifeq ($(BUILD),debug)
  CXXFLAGS  := $(CXXSTD) $(WARNINGS) $(INCLUDES) -g -O0
else ifeq ($(BUILD),use)
  CXXFLAGS  := $(CXXSTD) $(WARNINGS) $(INCLUDES) -O3
else
  $(error Unknown BUILD mode '$(BUILD)'. Use BUILD=debug or BUILD=use)
endif

# Example-specific flags (always debug)
EX_CXXFLAGS := $(CXXSTD) $(WARNINGS) $(INCLUDES) -g -O0

LDFLAGS     := -lm

# Sources and objects
SRC_FILES := $(shell find $(SRC_DIR) -name '*.cpp')
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRC_FILES))
MAIN_OBJ := $(BUILD_DIR)/main.o

# Examples
EXAMPLES_DIR := examples
EX_OBJ_DIR   := $(BUILD_DIR)/examples
EX_SRC_FILES := $(wildcard $(EXAMPLES_DIR)/*.cpp)
EX_BINS      := $(patsubst $(EXAMPLES_DIR)/%.cpp,$(BINS_DIR)/%,$(EX_SRC_FILES))
EX_OBJ_FILES := $(patsubst $(EXAMPLES_DIR)/%.cpp,$(EX_OBJ_DIR)/%.o,$(EX_SRC_FILES))

# compile_commands.json
COMPILE_COMMANDS := compile_commands.json

# Default target
.PHONY: all
all: $(COMPILE_COMMANDS) $(TARGET)

# Generate compile_commands.json
$(COMPILE_COMMANDS): $(SRC_FILES) main.cpp
	@echo "Generating $(COMPILE_COMMANDS)..."
	@echo "[" > $(COMPILE_COMMANDS)
	@for src in $(SRC_FILES); do \
		dir=$$(dirname $$src); \
		file=$$(basename $$src); \
		echo "  {" >> $(COMPILE_COMMANDS); \
		echo "    \"directory\": \"$$(pwd)\"," >> $(COMPILE_COMMANDS); \
		echo "    \"command\": \"$(CXX) $(CXXFLAGS) -c $$src -o $(BUILD_DIR)/$${src%.cpp}.o\"," >> $(COMPILE_COMMANDS); \
		echo "    \"file\": \"$$src\"" >> $(COMPILE_COMMANDS); \
		echo "  }," >> $(COMPILE_COMMANDS); \
	done
	@echo "  {" >> $(COMPILE_COMMANDS); \
	echo "    \"directory\": \"$$(pwd)\"," >> $(COMPILE_COMMANDS); \
	echo "    \"command\": \"$(CXX) $(CXXFLAGS) -c main.cpp -o $(BUILD_DIR)/main.o\"," >> $(COMPILE_COMMANDS); \
	echo "    \"file\": \"main.cpp\"" >> $(COMPILE_COMMANDS); \
	echo "  }" >> $(COMPILE_COMMANDS); \
	echo "]" >> $(COMPILE_COMMANDS)

# Link main target
$(TARGET): $(MAIN_OBJ) $(OBJ_FILES)
	$(CXX) $^ -o $@ $(LDFLAGS)

# Compile main sources
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile main.cpp (using SRC_DIR rule)
$(MAIN_OBJ): main.cpp | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile examples WITH DEBUG FLAGS ONLY
$(EX_OBJ_DIR)/%.o: $(EXAMPLES_DIR)/%.cpp | $(EX_OBJ_DIR)
	@mkdir -p $(dir $@)
	$(CXX) $(EX_CXXFLAGS) -c $< -o $@

# Link example binaries (in bins directory)
$(BINS_DIR)/%: $(EX_OBJ_DIR)/%.o $(OBJ_FILES)
	@mkdir -p $(dir $@)
	$(CXX) $^ -o $@ $(LDFLAGS)

# Ensure build directories exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(EX_OBJ_DIR):
	mkdir -p $(EX_OBJ_DIR)

# Convenience targets
.PHONY: debug use clean examples

debug:
	$(MAKE) BUILD=debug

use:
	$(MAKE) BUILD=use

examples: $(EX_BINS)

clean:
	-rm -rf $(TARGET) $(EX_BINS)
	-@rm -rf $(BUILD_DIR)/* $(BUILD_DIR)/.[!.]* $(BUILD_DIR)/..?* 2>/dev/null || true

# Unit Tests
TEST_OBJ_DIR := $(BUILD_DIR)/tests
CATCH_OBJ    := $(TEST_OBJ_DIR)/catch_lib.o
CATCH_SRC    := tests/catch_lib.cpp

$(TEST_OBJ_DIR):
	mkdir -p $(TEST_OBJ_DIR)

$(CATCH_OBJ): $(CATCH_SRC) | $(TEST_OBJ_DIR)
	@echo "Compiling Catch2 core (one-time setup)..."
	$(CXX) $(CXXSTD) -Itests -w -c $< -o $@

.PHONY: run_tests
run_tests: $(OBJ_FILES) $(CATCH_OBJ)
	@echo "Running tests..."
	@failed=0; \
	for test in tests/test_*.cpp; do \
		if [ -f "$$test" ]; then \
			exe="tests/$$(basename $$test .cpp)"; \
			$(CXX) $(CXXSTD) -Itests $(INCLUDES) $$test $(CATCH_OBJ) $(OBJ_FILES) $(LDFLAGS) -o $$exe && \
			./$$exe; \
			if [ $$? -eq 0 ]; then \
				echo "$$test passed ✓"; \
			else \
				echo "$$test failed ✗"; \
				failed=1; \
			fi; \
			rm -f $$exe; \
		fi; \
	done; \
	if [ $$failed -eq 1 ]; then \
		echo "Some tests failed! ✗"; \
		exit 1; \
	else \
		echo "All tests passed! ✓"; \
	fi
