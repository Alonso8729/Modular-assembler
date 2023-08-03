#ifndef __ASSEMBLER_H_
#define __ASSEMBLER_H_
#include "../data_structures/common_structs/common.h"
#include "../data_structures/dynamic_array/dynamic_array.h"
#include "../data_structures/trie/trie.h"
#include "../line_structure/lineToStructure.h"
#include "../preprocessor/preprocessor.h"
#include "../output_files/output_files.h"
#define OP_MAX_NUM 2
#define BASE_ADDRESS 100
#define MAX_MEMORY 1024
#define MACHINE_WORD_SIZE 12


/**
 * @brief 
 * 
 * @param file_count 
 * @param file_names 
 * @return int 
 */
int compile_program(int file_count, char **file_names);

#endif