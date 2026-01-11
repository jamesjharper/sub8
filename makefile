# Compiler and flags
CXX      ?= c++
CXX_FLAGS ?= -std=c++17 -fno-rtti -Wall -Wextra -Werror -O1 -g 
CXX_SANFLAGS?= -fsanitize=address,undefined -fno-omit-frame-pointer


# Include paths:
#  - include/      : your library headers (lora_db.hpp, etc.)
#  - /opt/homebrew/include or /usr/local/include: typical Catch2 installs
INCLUDE_DIR := include
INCLUDES    := -I$(INCLUDE_DIR) -I/opt/homebrew/include -I/usr/local/include

# Library paths for Catch2 (override if installed somewhere else)
CATCH2_LIBDIR1 ?= /opt/homebrew/lib
CATCH2_LIBDIR2 ?= /usr/local/lib

LDFLAGS   += -L$(CATCH2_LIBDIR1) -L$(CATCH2_LIBDIR2)

# Catch2 v3 static libs: libCatch2Main.a + libCatch2.a
LDLIBS    += -lCatch2Main -lCatch2 

# Directories
TEST_DIR   := tests
SRC_DIR    := src
BUILD_DIR  := build
OBJ_DIR    := $(BUILD_DIR)/obj
TARGET     := $(BUILD_DIR)/loradb_tests

# Source files
LIB_SRCS   := $(wildcard $(SRC_DIR)/*.cpp)
TEST_SRCS  := $(wildcard $(TEST_DIR)/*.cpp)

# All sources to build into the test binary
SRCS       := $(LIB_SRCS) $(TEST_SRCS)

OBJS := $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRCS))

# clang-format config
CLANG_FORMAT ?= clang-format

# clang-format files
FORMAT_EXTENSIONS := \
    -name "*.cpp" -o \
    -name "*.h"   -o \
    -name "*.hpp"

FORMAT_DIRS := . tests

.PHONY: all run clean format



test: clean run

# Default target: build the test binary
all: $(TARGET)

# Link step
$(TARGET): $(OBJS)
	@echo "Linking: $@"
	$(CXX) $(CXX_FLAGS) $(CXX_SANFLAGS) $(INCLUDES) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

# Compile any .cpp file into corresponding .o under build/obj/
# Pattern:
#   src/foo.cpp   -> build/obj/src/foo.o
#   tests/bar.cpp -> build/obj/tests/bar.o
$(OBJ_DIR)/%.o: %.cpp
	@echo "Compiling: $<"
	@mkdir -p $(dir $@)
	$(CXX) $(CXX_FLAGS) $(CXX_SANFLAGS) $(INCLUDES) -c $< -o $@

# Run tests
run: $(TARGET)
	@echo "Running tests..."
	@$(TARGET)

# Clean build artifacts
clean:
	@echo "Cleaning..."
	@rm -rf $(BUILD_DIR)

generate_luts:
	@echo "Generating Text Look up table(s)..."
	python3 sub8_strings_b5_lut.py encode b5_string_table.tsv sub8_strings_b5_encoding_lut.h
	python3 sub8_strings_b5_lut.py decode b5_string_table.tsv sub8_strings_b5_decoding_lut.h

format:
	@echo "Running clang-format (in-place)..."
	@find $(FORMAT_DIRS) \( $(FORMAT_EXTENSIONS) \) -print | xargs $(CLANG_FORMAT) -i