#ifndef __ASSEMBLER_H_
#define __ASSEMBLER_H_
#include "../preprocessor/preprocessor.h"
#include "/mnt/d/Documents/VSC Projects/mmn14/lexer.h"
#include "../data_structures/table/table.h"
#include "../data_structures/trie/trie.h"
#define OP_MAX_NUM 2
#define BASE_ADDRESS 100
/**
 * @brief 
 * 
 */
struct object_file{
    d_arr symbol_table;
    Trie symbol_search;
    d_arr code_image;
    d_arr data_image;
    d_arr extern_table;
};

typedef struct object_file* obj_file;

struct symbol{
    char symbol_name[MAX_NAME_SIZE];
    unsigned short address;
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


struct extern_table{
    char extern_name[MAX_NAME_SIZE];
    d_arr call_address;
};
/* not sure if needed
struct extern_symbol{
    char extern_name[MAX_NAME_SIZE];
    d_arr call_address;
};  */

typedef struct machine_word{
    unsigned short binary_code;
} machine_word;

/**
 * @brief Create a symbol object
 * 
 * @param sym_name 
 * @param address 
 * @param symbol_type 
 * @param declared_line 
 * @return struct symbol* 
 */
struct symbol* create_symbol(char sym_name[MAX_NAME_SIZE], unsigned short address ,int symbol_type, int declared_line);

/**
 * @brief Create a machine word object
 * 
 * @param binary_code 
 * @return machine_word* 
 */
machine_word* create_machine_word(unsigned short binary_code);
/**
 * @brief 
 * 
 * @param file_count 
 * @param file_names 
 * @return object_file 
 */

/**
 * @brief Create an object file object
 * 
 * @return struct object_file 
 */
obj_file create_obj_file();

/**
 * @brief 
 * 
 */

/**
 * @brief 
 * 
 * @param ext_table 
 */
void destroy_extern_table(d_arr* ext_table);

/**
 * @brief deallocate the memory of all the data structures inside the object
 * 
 */
void free_obj_file(obj_file);

/**
 * @brief 
 * 
 * @param file_count 
 * @param file_names 
 * @return struct object_file 
 */
struct object_file compile_program(int file_count,char** file_names);

/**
 * @brief 
 * 
 * @param obj 
 * @return int return 0 if there is no error, otherwise return 1
 */
int check_entry_definition(obj_file obj);




#endif