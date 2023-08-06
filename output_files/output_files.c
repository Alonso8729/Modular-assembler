#include "output_files.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief
 *
 * @param obj
 * @param file_name
 */
static void ent_file_output(obj_file obj, const char *file_name) {
  char *ent_file_name;
  int file_name_length;
  file_name_length = strlen(file_name);
  ent_file_name = malloc(file_name_length + strlen(ENT_FILE) +
                         1); /*adding +1 for null terminator*/
  strcpy(ent_file_name, file_name);
  strcat(ent_file_name, ENT_FILE);
  FILE *ent_file;
  ent_file = fopen(ent_file_name, "w");
  if (!ent_file) {
    printf("Failed to open ent_file\n");
    free(ent_file_name);
    return;
  }
  /*print entry symbol's names and their call addresses*/
  int sym_table_size;
  sym_table_size = get_item_count(obj->symbol_table);
  int i;
  struct symbol *curr_sym;
  for (i = 0; i < sym_table_size; i++) {
    curr_sym = get_item(obj->symbol_table, i);
    if (curr_sym->symbol_types == entry_code ||
        curr_sym->symbol_types == entry_data) {
      fprintf(ent_file, "%s\t%d\n", curr_sym->symbol_name, curr_sym->address);
    }
  }
  free(ent_file_name);
  fclose(ent_file);
}

/**
 * @brief
 *
 * @param obj
 * @param file_name
 */
static void ext_file_output(obj_file obj, const char *file_name) {
  /*handling .ext file creation*/
  char *ext_file_name;
  int file_name_length;
  file_name_length = strlen(file_name);
  ext_file_name = malloc(file_name_length + strlen(EXT_FILE) +
                         1); /*adding +1 for null terminator*/
  strcpy(ext_file_name, file_name);
  strcat(ext_file_name, EXT_FILE);
  FILE *ext_file;
  ext_file = fopen(ext_file_name, "w");
  if (!ext_file) {
    printf("Failed to open ext_file\n");
    free(ext_file_name);
    return;
  }
  /*print extern symbols names and  calls*/
  int ext_table_size;
  ext_table_size = get_item_count(obj->extern_table);
  int i;
  int j;
  extern_symbol *curr_ext;
  int *address;
  int call_size; /*how many times an extern symbol has been called*/
  for (i = 0; i < ext_table_size; i++) {
    curr_ext = get_item(obj->extern_table, i);
    call_size = get_item_count(curr_ext->call_address);
    for (j = 0; j < call_size; j++) {
      address = get_item(curr_ext->call_address, j);
      fprintf(ext_file, "%s\t%d\n", curr_ext->extern_name, *address);
    }
  }

  free(ext_file_name);
  fclose(ext_file);
}

/**
 * @brief
 *
 * @param decimal
 * @param base64
 */

static void decimal_to_base64(int decimal, char *base64) {
  char base64_chars[] =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  base64[0] = base64_chars[(decimal >> 6) & 0x3F]; /*0x3F used for masking*/
  base64[1] = base64_chars[decimal & 0x3F];
  base64[2] = '\0'; /* null terminator */
}

/** create .ob file with the size of data and code images, and base64 converted
 * machine words taken from code and data images
 * @brief
 *
 * @param obj
 * @param file_name
 */
static void ob_file_output(obj_file obj, const char *file_name) {
  /*handling .ob file creation*/
  char *ob_file_name;
  int file_name_length;
  file_name_length = strlen(file_name);
  ob_file_name = malloc(file_name_length + strlen(OB_FILE) +
                        1); /*adding +1 for null terminator*/
  strcpy(ob_file_name, file_name);
  strcat(ob_file_name, OB_FILE);
  FILE *ob_file;
  ob_file = fopen(ob_file_name, "w");
  if (!ob_file) {
    printf("Failed to open ob_file\n");
    free(ob_file_name);
    return;
  }
  /*print to ob file the number of machine words in code section and data
   * section*/
  int code_size;
  code_size = get_item_count(obj->code_image);
  int data_size;
  data_size = get_item_count(obj->data_image);
  fprintf(ob_file, "%d %d\n", code_size, data_size);

  /*print to ob file the machine words converted to base 64*/
  /*first printing code image and then data image*/
  int i;
  machine_word *m_word;
  int binary;
  for (i = 0; i < code_size; i++) {
    char base64[3]; /*a string with two chars for the base 64 representation and
                       third char for null terminator*/
    m_word = get_item(obj->code_image, i);
    binary = m_word->binary_code;
    decimal_to_base64(binary, base64);
    fprintf(ob_file, "%s\n", base64);
  }
  for (i = 0; i < data_size; i++) {
    char base64[3];
    m_word = get_item(obj->data_image, i);
    binary = m_word->binary_code;
    decimal_to_base64(binary, base64);
    fprintf(ob_file, "%s\n", base64);
  }
  free(ob_file_name);
  fclose(ob_file);
}

/**
 * @brief check if there are entries in the symbol table
 *
 * @param obj
 * @return 1 if entry was found, otherwise return 0
 */
static int check_if_entry_exists(obj_file obj) {
  int i;
  int sym_table_size;
  sym_table_size = get_item_count(obj->symbol_table);
  struct symbol *curr_sym;
  for (i = 0; i < sym_table_size; i++) {
    curr_sym = get_item(obj->symbol_table, i);
    if (curr_sym->symbol_types == entry_code ||
        curr_sym->symbol_types == entry_data) /*entry was found*/
      return 1;
  }
  return 0;
}

void output_all_files(obj_file obj, const char *file_name) {
  /*if there are entries, make .ent file*/
  if (check_if_entry_exists(obj))
    ent_file_output(obj, file_name);
  /*if there are extern symbols, make .ext file*/
  if (get_item_count(obj->extern_table) > 0)
    ext_file_output(obj, file_name);
  /*make .ob file*/
  ob_file_output(obj, file_name);
}
