#include "assembler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct symbol *create_symbol(char sym_name[MAX_NAME_SIZE],
                             unsigned short address, int symbol_type,
                             int declared_line) {
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

int check_entry_definition(obj_file obj) {
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

machine_word *create_machine_word(unsigned short binary_code) {
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
  new_obj->symbol_search = create_trie();
  new_obj->code_image = create_dynamic_array(sizeof(machine_word));
  new_obj->data_image = create_dynamic_array(sizeof(machine_word));
  new_obj->extern_table = create_dynamic_array(sizeof(struct extern_table));
  return new_obj;
}

void destroy_machine_word(machine_word *m_word) { free(m_word); }

void destroy_extern_table(d_arr* ext_table) {
  if (ext_table) {
    int extern_table_size = get_item_count(*ext_table);
    int i;
    for(i=0; i<extern_table_size; i++){
      struct extern_table* curr_ext = get_item(*ext_table,i);
      if(curr_ext){
        destroy_dynamic_array(curr_ext ->call_address);
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
  destroy_dynamic_array(obj->extern_table);
  free(obj);
}
