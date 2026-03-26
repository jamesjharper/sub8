# -----------------------------
# Tooling
# -----------------------------
CXX       ?= c++
PYTHON    ?= python3

CXX_FLAGS    ?= -std=c++17 -fno-rtti -Wall -Wextra -Werror -O1 -g
CXX_SANFLAGS ?= -fsanitize=address,undefined -fno-omit-frame-pointer

# Catch2 libs (override if needed)
CATCH2_LIBDIR1 ?= /opt/homebrew/lib
CATCH2_LIBDIR2 ?= /usr/local/lib
LDFLAGS   += -L$(CATCH2_LIBDIR1) -L$(CATCH2_LIBDIR2)
LDLIBS    += -lCatch2Main -lCatch2

# -----------------------------
# Repo paths
# -----------------------------
REPO_ROOT   := .
SRC_PY_DIR  := src

CPP_LIB_DIR      := src/languages/cpp/src
CPP_TEST_DIR     := src/languages/cpp/tests
CPP_EXAMPLE_DIR  := src/languages/cpp/example
CPP_EXAMPLE_TEST := $(CPP_EXAMPLE_DIR)/tests/example_tests.cpp
CPP_EXAMPLE_YAML := $(CPP_EXAMPLE_DIR)/example.yaml

# -----------------------------
# Build dirs
# -----------------------------
BUILD_DIR := build
OBJ_DIR   := $(BUILD_DIR)/cpp/obj
TARGET    := $(BUILD_DIR)/cpp/sub8_tests

# -----------------------------
# Generated outputs (from pipeline)
# NOTE: your repo tree shows: src/languages/cpp/example/code_gen/generated/example.{h,cpp}
# -----------------------------
GEN_OUT_ROOT := $(CPP_EXAMPLE_DIR)/code_gen
GEN_DIR      := $(GEN_OUT_ROOT)/generated
GEN_H        := $(GEN_DIR)/example.h
GEN_CPP      := $(GEN_DIR)/example.cpp

# Incremental generation stamp
GEN_STAMP := $(BUILD_DIR)/gen/example.stamp

# -----------------------------
# Includes
# -----------------------------
INCLUDES := -I$(CPP_LIB_DIR)
INCLUDES += -I$(GEN_DIR)
INCLUDES += -I/opt/homebrew/include -I/usr/local/include

# -----------------------------
# C++ build inputs
# -----------------------------
CPP_LIB_SRCS        := $(wildcard $(CPP_LIB_DIR)/*.cpp)
CPP_TEST_SRCS       := $(wildcard $(CPP_TEST_DIR)/*.cpp)
CPP_EXTRA_TEST_SRCS := $(CPP_EXAMPLE_TEST)
CPP_GEN_SRCS        := $(GEN_CPP)

CPP_SRCS := $(CPP_LIB_SRCS) $(CPP_TEST_SRCS) $(CPP_EXTRA_TEST_SRCS) $(CPP_GEN_SRCS)

OBJS := $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(CPP_SRCS))
EXPANDED := $(patsubst %.cpp,$(OBJ_DIR)/%.i,$(CPP_SRCS))

# -----------------------------
# Formatting
# -----------------------------
CLANG_FORMAT ?= clang-format
FORMAT_EXTENSIONS := \
    -name "*.cpp" -o \
    -name "*.h"   -o \
    -name "*.hpp"

FORMAT_DIRS := src

# -----------------------------
# Phonies
# -----------------------------
.PHONY: all test py-test cpp-test cpp-compile cpp-link run clean format expand gen

all: cpp-test

# -----------------------------
# Python tests
# -----------------------------
py-test:
	@echo "Running python tests..."
	@PYTHONPATH=./src $(PYTHON) -m pytest -q

# -----------------------------
# Codegen
# -----------------------------
gen: $(GEN_STAMP)
	@echo "Generated: $(GEN_H)"
	@echo "Generated: $(GEN_CPP)"

# This stamp makes codegen incremental: only re-run when inputs change.
$(GEN_STAMP): \
	$(CPP_EXAMPLE_YAML) \
	$(SRC_PY_DIR)/pipeline.py \
	$(SRC_PY_DIR)/pipeline_core.py \
	$(SRC_PY_DIR)/merge.py \
	$(SRC_PY_DIR)/op_gen.py \
	$(SRC_PY_DIR)/code_gen.py
	@echo "Pipeline all (in-memory) -> $(GEN_OUT_ROOT)/"
	@mkdir -p "$(BUILD_DIR)/gen"
	@mkdir -p "$(GEN_OUT_ROOT)"
	@$(PYTHON) $(SRC_PY_DIR)/pipeline.py all "$(CPP_EXAMPLE_YAML)" --repo-root "$(REPO_ROOT)" -o "$(GEN_OUT_ROOT)/"
	@# ensure expected generated outputs exist
	@test -f "$(GEN_H)" && test -f "$(GEN_CPP)"
	@touch "$(GEN_STAMP)"
# -----------------------------
# C++ build inputs
# -----------------------------
CPP_LIB_SRCS        := $(wildcard $(CPP_LIB_DIR)/*.cpp)
CPP_TEST_SRCS       := $(wildcard $(CPP_TEST_DIR)/*.cpp)

# Example test + generated TU
CPP_EXAMPLE_TEST_SRCS := $(CPP_EXAMPLE_TEST)
CPP_GEN_SRCS          := $(GEN_CPP)

# -----------------------------
# Build dirs / targets
# -----------------------------
BUILD_DIR := build
OBJ_DIR   := $(BUILD_DIR)/cpp/obj

TARGET_CORE    := $(BUILD_DIR)/cpp/sub8_tests
TARGET_EXAMPLE := $(BUILD_DIR)/cpp/example_tests

# Core test program: library + core tests (NO generated example.cpp)
CORE_SRCS := $(CPP_LIB_SRCS) $(CPP_TEST_SRCS)
CORE_OBJS := $(patsubst %.cpp,$(OBJ_DIR)/core/%.o,$(CORE_SRCS))

# Example test program: generated example.cpp + example_tests.cpp (NO library src/*.cpp)
EXAMPLE_SRCS := $(CPP_GEN_SRCS) $(CPP_EXAMPLE_TEST_SRCS)
EXAMPLE_OBJS := $(patsubst %.cpp,$(OBJ_DIR)/example/%.o,$(EXAMPLE_SRCS))

# -----------------------------
# C++ build
# -----------------------------
cpp-test: $(TARGET_CORE) $(TARGET_EXAMPLE)
	@echo "Running core C++ tests..."
	@$(TARGET_CORE)
	@echo "Running generated example tests..."
	@$(TARGET_EXAMPLE)

cpp-compile: $(CORE_OBJS) $(EXAMPLE_OBJS)
	@echo "Compiled core + example objects."

cpp-link: $(TARGET_CORE) $(TARGET_EXAMPLE)
	@true

$(TARGET_CORE): $(CORE_OBJS)
	@echo "Linking: $@"
	@mkdir -p $(dir $@)
	$(CXX) $(CXX_FLAGS) $(CXX_SANFLAGS) $(INCLUDES) $(LDFLAGS) -o $@ $(CORE_OBJS) $(LDLIBS)

$(TARGET_EXAMPLE): $(EXAMPLE_OBJS)
	@echo "Linking: $@"
	@mkdir -p $(dir $@)
	$(CXX) $(CXX_FLAGS) $(CXX_SANFLAGS) $(INCLUDES) $(LDFLAGS) -o $@ $(EXAMPLE_OBJS) $(LDLIBS)

# Compile core objects
$(OBJ_DIR)/core/%.o: %.cpp $(GEN_STAMP)
	@echo "Compiling (core): $<"
	@mkdir -p $(dir $@)
	$(CXX) $(CXX_FLAGS) $(CXX_SANFLAGS) $(INCLUDES) -c $< -o $@

# Compile example objects
$(OBJ_DIR)/example/%.o: %.cpp $(GEN_STAMP)
	@echo "Compiling (example): $<"
	@mkdir -p $(dir $@)
	$(CXX) $(CXX_FLAGS) $(CXX_SANFLAGS) $(INCLUDES) -c $< -o $@

# -----------------------------
# Meta targets
# -----------------------------
test: py-test cpp-test

clean:
	@echo "Cleaning..."
	@rm -rf $(BUILD_DIR)

format:
	@echo "Running clang-format (in-place)..."
	@find $(FORMAT_DIRS) \( $(FORMAT_EXTENSIONS) \) -print | xargs $(CLANG_FORMAT) -i
