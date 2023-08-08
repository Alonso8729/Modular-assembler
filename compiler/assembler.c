#include "assembler.h"
#include "first_pass.h"
#include "second_pass.h"

/**
 * @brief check if all the entries that has been declared was used
 *
 * @param obj
 * @return int return 0 if there is no error, otherwise return 1
 */
static int check_entry_definition(obj_file obj) {
  int error = 0;
  int i;
  struct symbol *sym;
  int size;
  size = obj->symbol_table->item_count;
  for (i = 0; i < size; i++) {
    sym = get_item(obj->symbol_table, i);
    if (sym->symbol_types == entry) {
      error = 1;
      printf("Entry label %s was declared in line %d as entry but was never "
             "defined\n",
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

void add_to_extern_table(obj_file *obj, int address, char *extern_name) {
  int i;
  obj_file curr_obj;
  int table_size;
  extern_symbol *new_ext;
  struct extern_table *curr_ext;
  curr_obj = *obj;
  table_size = get_item_count(curr_obj->extern_table);

  /*search for extern name in the extern table*/
  for (i = 0; i < table_size; i++) {
    curr_ext = get_item(curr_obj->extern_table, i);
    if (strcmp(extern_name, curr_ext->extern_name) == 0) {
      insert_item(curr_ext->call_address, &address);
      return;
    }
  }
  /*adding a new item to the extern table*/
  new_ext = create_extern_symbol(extern_name, address);
  new_ext->call_address = create_dynamic_array(sizeof(int));
  insert_item(new_ext->call_address, &address);
  insert_item(curr_obj->extern_table, new_ext);
  return;
}

int compile_program(int file_count, char **file_names) {
  FILE *am_file;
  const char *am_file_name;
  int i;
  obj_file curr_obj;
  int entry_check;
  int memory_check;

  for (i = 0; i < file_count; i++) {
    am_file_name = preprocess(file_names[i]);
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
              output_all_files(curr_obj, file_names[i]);
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