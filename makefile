# Makefile

# Compiler
CC = gcc

# Compiler Flags
CFLAGS = -g -Wall -ansi -pedantic 

# Directories
SRC_DIR = .
COMPILER_DIR = compiler
PREPROCESS_DIR = preprocessor
LINE_STRUCT_DIR = line_structure
DATA_STRUCT_DIR = data_structures
COMMON_DIR = data_structures/common_structs
TRIE_DIR = data_structures/trie
DYNAMIC_ARRAY_DIR = data_structures/dynamic_array
OUTPUT_DIR = output_files

# Source Files
SRC = $(COMPILER_DIR)/assembler.c $(PREPROCESS_DIR)/preprocessor.c $(COMPILER_DIR)/main.c \
      $(LINE_STRUCT_DIR)/line_to_structure.c $(COMPILER_DIR)/first_pass.c $(COMPILER_DIR)/second_pass.c\
      $(COMMON_DIR)/common.c $(TRIE_DIR)/trie.c $(DYNAMIC_ARRAY_DIR)/dynamic_array.c\
	  $(OUTPUT_DIR)/output_files.c

# Header Files
INCLUDE = -I$(SRC_DIR) -I$(COMPILER_DIR) -I$(PREPROCESS_DIR) -I$(LINE_STRUCT_DIR) \
          -I$(DATA_STRUCT_DIR) -I$(COMMON_DIR) -I$(TRIE_DIR) -I$(DYNAMIC_ARRAY_DIR)

# Output Executable
TARGET = assembler

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(INCLUDE) $^ -o $@ -lm

clean:
	rm -f $(TARGET)






