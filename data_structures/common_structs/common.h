#ifndef _COMMON_H
#define _COMMON_H
#include "../dynamic_array/dynamic_array.h"
#include "../trie/trie.h"
#define MAX_LABEL_LEN 31
/**
 * @brief
 *
 */
struct object_file {
  d_arr symbol_table;
  Trie symbol_search;
  d_arr code_image;
  d_arr data_image;
  d_arr extern_table;
};

typedef struct object_file *obj_file;

struct symbol {
  char symbol_name[MAX_LABEL_LEN + 1];
  int address;
  int declared_line;
  enum { entry, external, code, data, entry_code, entry_data } symbol_types;
};

/*symbols that have been declared before being defined*/

struct extern_table {
  char extern_name[MAX_LABEL_LEN + 1];
  d_arr call_address;
};

typedef struct extern_symbol {
  char extern_name[MAX_LABEL_LEN + 1];
  d_arr call_address;
} extern_symbol;

typedef struct machine_word {
  int binary_code;
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
struct symbol *create_symbol(char sym_name[MAX_LABEL_LEN + 1], int address,
                             int symbol_type, int declared_line);

/**
 * @brief Create a machine word object
 *
 * @param binary_code
 * @return machine_word*
 */
machine_word *create_machine_word(int binary_code);
/**
 * @brief
 *
 * @param file_count
 * @param file_names
 * @return object_file
 */

/**
 * @brief Create a extern symbol object
 *
 * @param sym_name
 * @param address
 * @return struct extern_symbol*
 */
struct extern_symbol *create_extern_symbol(char sym_name[MAX_LABEL_LEN + 1],
                                           int address);

/**
 * @brief print code_image and data image number of machine words, and
 * the code image and data image machine words accordingly
 *
 * @param obj
 */

void print_machine_words(obj_file obj);

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
void destroy_extern_table(d_arr *ext_table);

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

#endif