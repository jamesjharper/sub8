# Minimal Makefile for src/languages/cpp/example
# Builds ONLY the generated example + tests into a standalone test binary.
#
# If/when codegen truly inlines *all* required sub8 implementation into
# code_gen/generated/example.cpp, set:
#   make INLINE_ALL=1
# to avoid compiling any library sources.

CXX ?= c++

CXX_FLAGS    ?= -std=c++17 -fno-rtti -Wall -Wextra -Werror -O1 -g
CXX_SANFLAGS ?= -fsanitize=address,undefined -fno-omit-frame-pointer

# Catch2 libs (override if needed)
CATCH2_LIBDIR1 ?= /opt/homebrew/lib
CATCH2_LIBDIR2 ?= /usr/local/lib
LDFLAGS   += -L$(CATCH2_LIBDIR1) -L$(CATCH2_LIBDIR2)
LDLIBS    += -lCatch2Main -lCatch2

# -------- paths (relative to this folder) --------
EXAMPLE_DIR := .
GEN_DIR     := $(EXAMPLE_DIR)/code_gen/generated
TEST_DIR    := $(EXAMPLE_DIR)/tests

# If generated example.cpp has not fully inlined sub8 implementation, we compile
# the needed .cpp files from the library directory *except* sub8_strings.cpp,
# because generated example.cpp already inlines it (and its LUTs).
INLINE_ALL ?= 0
SUB8_LIB_DIR := ../src
#SUB8_LIB_FALLBACK_SRCS := \
#  $(SUB8_LIB_DIR)/sub8_primitives.cpp \
#  $(SUB8_LIB_DIR)/sub8_floats.cpp \
#  $(SUB8_LIB_DIR)/sub8_variant.cpp

SUB8_LIB_FALLBACK_SRCS := 

# -------- build outputs --------
BUILD_DIR := build
OBJ_DIR   := $(BUILD_DIR)/obj
TARGET    := $(BUILD_DIR)/example_tests

# -------- includes --------
INCLUDES := -I$(GEN_DIR) -I$(SUB8_LIB_DIR)
INCLUDES += -I/opt/homebrew/include -I/usr/local/include

# -------- sources --------
GEN_SRCS  := $(GEN_DIR)/example.cpp
TEST_SRCS := $(TEST_DIR)/example_tests.cpp

ifeq ($(INLINE_ALL),1)
  SRCS := $(GEN_SRCS) $(TEST_SRCS)
else
  SRCS := $(GEN_SRCS) $(TEST_SRCS) $(SUB8_LIB_FALLBACK_SRCS)
endif

OBJS := $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRCS))

.PHONY: all test clean print-config

all: test

test: $(TARGET)
	@echo "Running example tests..."
	@./$(TARGET)

print-config:
	@echo "INLINE_ALL=$(INLINE_ALL)"
	@echo "SRCS=$(SRCS)"

$(TARGET): $(OBJS)
	@echo "Linking: $@"
	@mkdir -p $(dir $@)
	$(CXX) $(CXX_FLAGS) $(CXX_SANFLAGS) $(INCLUDES) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

$(OBJ_DIR)/%.o: %.cpp
	@echo "Compiling: $<"
	@mkdir -p $(dir $@)
	$(CXX) $(CXX_FLAGS) $(CXX_SANFLAGS) $(INCLUDES) -c $< -o $@

clean:
	@echo "Cleaning $(BUILD_DIR)..."
	@rm -rf $(BUILD_DIR)
