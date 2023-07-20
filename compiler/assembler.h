#ifndef __ASSEMBLER_H_
#define __ASSEMBLER_H_
#include "../preprocessor/preprocessor.h"
#include "/mnt/d/Documents/VSC Projects/mmn14/lexer.h"
#include "../data_structures/table/table.h"
#include "../data_structures/trie/trie.h"
#define BASE_ADDRESS 100
/**
 * @brief 
 * 
 */
struct object_file{
    d_arr symbol_table;
    Trie symbol_search;
    d_arr code_section;
    d_arr data_section;
    d_arr extern_table;
    int entry_counter;
};

typedef struct object_file* obj_file;

struct symbol{
    char symbol_name[MAX_NAME_SIZE];
    unsigned int address;
    unsigned short declared_line;
    enum{
        entry,
        external,
        code,
        data,
        entry_code,
        entry_data
    }symbol_types;
};

/*symbols that have been declared before being defined*/
struct missing_sym_table{
    unsigned short call_address;
    unsigned short* binary_ptr;/*pointer to the missing address*/
    char symbol_name[MAX_NAME_SIZE];
};

struct extern_table{
    char extern_name[MAX_NAME_SIZE];
    d_arr extern_addresses;
};

/**
 * @brief 
 * 
 * @param file_count 
 * @param file_names 
 * @return object_file 
 */

/**
 * @brief Create a obj file object
 * 
 * @return struct object_file 
 */
struct object_file create_obj_file();

/**
 * @brief 
 * 
 */
void destroy_obj_file(struct object_file*);

/**
 * @brief 
 * 
 * @param file_count 
 * @param file_names 
 * @return struct object_file 
 */
struct object_file compile_program(int file_count,char** file_names);





#endif