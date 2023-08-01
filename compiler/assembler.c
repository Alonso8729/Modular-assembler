#include "assembler.h"
#include "first_pass.h"
#include "second_pass.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct symbol *create_symbol(char sym_name[MAX_LABEL_LEN + 1], short address,
                             int symbol_type, int declared_line) {
  struct symbol *new_symbol = malloc(sizeof(struct symbol));
  if (new_symbol) {
    strcpy(new_symbol->symbol_name, sym_name);
    new_symbol->symbol_types = symbol_type;
    new_symbol->declared_line = declared_line;
    new_symbol->address = address;
    return new_symbol;
  }
  printf("Memory allocation failed\n");
  exit(1);
}

struct extern_symbol *create_extern_symbol(char sym_name[MAX_LABEL_LEN + 1],
                                           short address) {
  extern_symbol *new_ext = malloc(sizeof(extern_symbol));
  if (new_ext) {
    strcpy(new_ext->extern_name, sym_name);
    new_ext->call_address = create_dynamic_array(sizeof(short));
    insert_item(new_ext->call_address, &address);
    return new_ext;
  }
  printf("Memory allocation failed\n");
  exit(1);
}

machine_word *create_machine_word(short binary_code) {
  machine_word *word = (machine_word *)malloc(sizeof(machine_word));
  if (!word) {
    printf("Memory allocation failed\n");
    exit(1);
  }
  word->binary_code = binary_code;
  return word;
}

obj_file create_obj_file() {
  obj_file new_obj = malloc(sizeof(struct object_file));
  new_obj->symbol_table = create_dynamic_array(sizeof(struct symbol));
  new_obj->symbol_search = NULL;
  new_obj->symbol_search = create_trie();
  new_obj->code_image = create_dynamic_array(sizeof(machine_word));
  new_obj->data_image = create_dynamic_array(sizeof(machine_word));
  new_obj->extern_table = create_dynamic_array(sizeof(struct extern_table));
  return new_obj;
}

void destroy_machine_word(obj_file obj) {
  if (obj) {
    int i;
    int data_image_size = get_item_count(obj->data_image);
    int code_image_size = get_item_count(obj->code_image);
    for (i = 0; i < code_image_size; i++) {
      machine_word *m = get_item(obj->code_image, i);
      free(m);
    }
    for (i = 0; i < data_image_size; i++) {
      machine_word *m = get_item(obj->data_image, i);
      free(m);
    }
  }
}

void destroy_extern_table(d_arr *ext_table) {
  if (ext_table) {
    int extern_table_size = get_item_count(*ext_table);
    int i;
    for (i = 0; i < extern_table_size; i++) {
      struct extern_table *curr_ext = get_item(*ext_table, i);
      if (curr_ext) {
        destroy_dynamic_array(curr_ext->call_address);
        free(curr_ext);
      }
    }
    destroy_dynamic_array(*ext_table);
  }
}

void free_obj_file(obj_file obj) {
  destroy_dynamic_array(obj->symbol_table);
  trie_destroy(obj->symbol_search);
  destroy_dynamic_array(obj->code_image);
  destroy_dynamic_array(obj->data_image);
  destroy_extern_table(&obj->extern_table);
  free(obj);
}

/**
 * @brief
 *
 * @param obj
 * @return int return 0 if there is no error, otherwise return 1
 */
static int check_entry_definition(obj_file obj) {
  int error = 0;
  int i;
  struct symbol *sym;
  int size = obj->symbol_table->item_count;
  for (i = 0; i < size; i++) {
    sym = get_item(obj->symbol_table, i);
    if (sym->symbol_types == entry) {
      error = 1;
      printf("Entry label %s was declared in line %d but was never defined\n",
             sym->symbol_name, sym->declared_line);
    }
  }
  return error;
}

/**
 * @brief
 *
 * @param total_m_words machine words counter
 * @return int return 1 if memory exceeded, otherwise return 0
 */
static int check_if_memory_exceeded(int total_m_words) {
  return total_m_words > MAX_MEMORY ? 1 : 0;
}

int compile_program(int file_count, char **file_names) {
  FILE *am_file;
  const char *am_file_name;
  int i;
  obj_file curr_obj;
  int entry_check;
  int memory_check;

  for (i = 0; i < file_count; i++) {
    am_file_name = preprocess(*(file_names + i));
    if (am_file_name) {
      am_file = fopen(am_file_name, "r");
      if (am_file) {
        curr_obj = create_obj_file();
        if (first_pass(
                am_file,
                curr_obj)) { /*check if first pass compilation succeeded*/
          entry_check = check_entry_definition(curr_obj);
          memory_check =
              check_if_memory_exceeded(get_item_count(curr_obj->code_image) +
                                       get_item_count(curr_obj->data_image));
          if (!entry_check &&
              !memory_check) { /*check if memory is not exceeded the memory
                                  limit, and all the entries has been defined*/
            rewind(am_file);
            if (second_pass(curr_obj, am_file)) {
              /*send to output files function*/
            }
            fclose(am_file);
            free_obj_file(curr_obj);
          }
        }
      }
      free((void *)am_file_name);
    }
  }
  return 0;
}