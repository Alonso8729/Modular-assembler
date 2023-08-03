#include "common.h"

struct symbol *create_symbol(char sym_name[MAX_LABEL_LEN + 1], int address,
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
                                           int address) {
  extern_symbol *new_ext = malloc(sizeof(extern_symbol));
  if (new_ext) {
    strcpy(new_ext->extern_name, sym_name);
    new_ext->call_address = create_dynamic_array(sizeof(int));
    insert_item(new_ext->call_address, &address);
    return new_ext;
  }
  printf("Memory allocation failed\n");
  exit(1);
}

machine_word *create_machine_word(int binary_code) {
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
  if (*ext_table) {
    int extern_table_size = get_item_count(*ext_table);
    int i;
    for (i = 0; i < extern_table_size; i++) {
      struct extern_table *curr_ext = get_item(*ext_table, i);
      if (curr_ext) {
        destroy_dynamic_array(curr_ext->call_address);
        free(curr_ext);
      }
    }
    free(*ext_table);
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

void print_machine_words(obj_file obj) {
  int i;
  int code_size = get_item_count(obj->code_image);
  int data_size = get_item_count(obj->data_image);
  machine_word *m_word;
  int binary;
  printf("%d %d\n", code_size, data_size);
  for (i = 0; i < code_size; i++) {
    m_word = get_item(obj->code_image, i);
    binary = m_word->binary_code;
    printf("%d\n", binary);
  }
  for (i = 0; i < data_size; i++) {
    m_word = get_item(obj->data_image, i);
    binary = m_word->binary_code;
    printf("%d\n", binary);
  }
}