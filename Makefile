# Libreria Ingegneria Software
# Anno Accademico 2025-2026 - Università di Roma La Sapienza

# Project settings
TARGET      := main
SRC_DIR     := src
INC_DIR     := include
BUILD_DIR   := build

# Tools
CXX         := g++
CXXSTD      := -std=c++17

# Build type (can be overridden: make BUILD=release)
BUILD       ?= release

# Common flags
WARNINGS    := -Wall -Wextra -Wpedantic
INCLUDES    := -I$(INC_DIR)

ifeq ($(BUILD),debug)
  CXXFLAGS  := $(CXXSTD) $(WARNINGS) $(INCLUDES) -g -O0 -DNDEBUG
else ifeq ($(BUILD),release)
  CXXFLAGS  := $(CXXSTD) $(WARNINGS) $(INCLUDES) -O3
else
  $(error Unknown BUILD mode '$(BUILD)'. Use BUILD=debug or BUILD=release)
endif

# Example-specific flags (always debug)
EX_CXXFLAGS := $(CXXSTD) $(WARNINGS) $(INCLUDES) -g -O0

LDFLAGS     :=

# Sources and objects
SRC_FILES := $(shell find $(SRC_DIR) -name '*.cpp')
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRC_FILES))
MAIN_OBJ := $(BUILD_DIR)/main.o

# Examples
EXAMPLES_DIR := examples
EX_OBJ_DIR   := $(BUILD_DIR)/examples
EX_SRC_FILES := $(wildcard $(EXAMPLES_DIR)/*.cpp)
EX_BINS      := $(patsubst $(EXAMPLES_DIR)/%.cpp,%,$(EX_SRC_FILES))
EX_OBJ_FILES := $(patsubst $(EXAMPLES_DIR)/%.cpp,$(EX_OBJ_DIR)/%.o,$(EX_SRC_FILES))

# compile_commands.json
COMPILE_COMMANDS := compile_commands.json

# Default target
.PHONY: all
all: $(COMPILE_COMMANDS) $(TARGET)

# Generate compile_commands.json
$(COMPILE_COMMANDS): $(SRC_FILES)
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
	@sed -i '$$s/,//' $(COMPILE_COMMANDS)
	@echo "]" >> $(COMPILE_COMMANDS)

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

# Link example binaries (in project root)
%: $(EX_OBJ_DIR)/%.o $(OBJ_FILES)
	$(CXX) $^ -o $@ $(LDFLAGS)

# Ensure build directories exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(EX_OBJ_DIR):
	mkdir -p $(EX_OBJ_DIR)

# Convenience targets
.PHONY: debug release clean examples

debug:
	$(MAKE) BUILD=debug

release:
	$(MAKE) BUILD=release

examples: $(EX_BINS)

clean:
	rm -rf $(BUILD_DIR) $(TARGET) $(EX_BINS)
