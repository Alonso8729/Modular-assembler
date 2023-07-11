/**
 * @file preprocessor.c
 * @brief Implementation of the preproccessor including macro spreading in a new
 * ".am" file
 * @version 0.1
 * @date 2023-06-16
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "preprocessor.h"
#include "../data_structures/table/table.h"
#include "../data_structures/trie/trie.h"
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define SKIP_SPACE(ptr)                                                        \
  while (isspace(*ptr))                                                        \
  ptr++
#define MAX_MCR_NAME 31
#define SPACE_CHARS "\t\f\n\r\v "
#define MAX_LINE 81
#define AS_FILE_EXT ".as"
#define AM_FILE_EXT ".am"

enum line_options {
  COMMENT_LINE,
  NULL_LINE,
  MACRO_DEF,
  MACRO_END,
  MACRO_CALL,
  REGULAR_LINE
};

struct macro {
  char mcr_name[1 + MAX_MCR_NAME]; /*extra char for '/0'*/
  d_arr lines;
};

typedef struct macro *Macro;

static void *create_macro(const char *m_name) {
  Macro mcr = malloc(sizeof(struct macro));
  if (!mcr) {
    printf("Memory allocation failed");
    return NULL;
  }
  strcpy(mcr->mcr_name, m_name);
  mcr->lines = create_dynamic_array(sizeof(void *));
  return mcr;
}

static void destroy_mcr(Macro m) {
  if (!m)
    return;
  Macro tmp = m;
  destroy_dynamic_array(tmp->lines);
  free(tmp);
}

static enum line_options line_detector(char *line, Macro *macro,
                                       Trie mcr_search, d_arr mcr_table) {
  char *token = line; /*char pointer to help us analyze line structure*/
  SKIP_SPACE(token);
  if (token[0] == '\0' || token[0] == '\n')
    return NULL_LINE;
  else if (token[0] == ';')
    return COMMENT_LINE;
  else if (strncmp(token, "mcro", 4) == 0 &&
           isspace(token[4])) { /*macro defination*/
    token += 4;
    SKIP_SPACE(token); /*skip to the macro name first character*/
    const char *mcro_name = token;
    /*find the end of the mcro_name*/
    token = strpbrk(mcro_name, SPACE_CHARS);
    /*eliminate white spaces from macro name*/
    if (token != NULL) {
      *token = '\0';
    }
    Macro m = create_macro(mcro_name);
    /*inserting the new macro to the macro_table and macro lookup trie*/
    insert_to_trie(m->mcr_name, mcr_search->root, insert_item(mcr_table, m));
    *macro = m;
    return MACRO_DEF;
  } else if (strncmp(token, "endmcro", 7) == 0 && isspace(token[7])) {
    *macro = NULL; /*flag off: we are not inside a macro definition*/
    return MACRO_END;
  }
  const char *mcro_name = token;
  /*find the end of the mcro_name*/
  token = strpbrk(mcro_name, SPACE_CHARS);
  /*ensure macro name has no white spaces*/
  if (!token) {
    *token = '\0';
  }
  void *is_mcr_exist = find_str(mcr_search->root, mcro_name);
  if (is_mcr_exist) { /*we find the macro*/
    *macro = is_mcr_exist;
    return MACRO_CALL;
  }
  return REGULAR_LINE;
}

const char *preprocess(const char *input_file_name) {
  /*variable declaration*/
  Macro *macro;
  Trie mcr_search;
  d_arr macro_table;
  /*appending the appropriate extensions for .as and.am files*/
  size_t file_name_len = strlen(input_file_name);
  char *as_file_name;
  as_file_name = malloc(file_name_len + strlen(AS_FILE_EXT) +
                        1); /*adding +1 for null terminator*/
  strcpy(as_file_name, input_file_name);
  strcat(as_file_name, AS_FILE_EXT);
  char *am_file_name;
  am_file_name = malloc(file_name_len + strlen(AM_FILE_EXT) + 1);
  strcpy(am_file_name, input_file_name);
  strcat(am_file_name, AM_FILE_EXT);

  /*open .as file for reading and .am file for writing*/
  FILE *as_file = fopen(as_file_name, "r");
  if (!as_file) {
    printf("Failed to open the file\n");
    free(as_file_name);
    return NULL;
  }
  FILE *am_file = fopen(am_file_name, "w");
  if (!am_file) {
    printf("Failed to open the file\n");
    fclose(as_file);
    return NULL;
  }
  /*creating data structures*/
  macro = NULL;
  mcr_search = create_trie();
  macro_table = create_dynamic_array(sizeof(Macro));
  if (!mcr_search || !macro_table) {
    printf("Failed to create data structures\n");
    destroy_dynamic_array(macro_table);
    trie_destroy(mcr_search);
    fclose(as_file);
    fclose(am_file);
    return NULL;
  }

  enum line_options line_type;
  char linebuffer[MAX_LINE] = {0};
  while (fgets(linebuffer, sizeof(linebuffer), as_file)) {
    line_type = line_detector(linebuffer, macro, mcr_search, macro_table);
    switch (line_type) {
    case COMMENT_LINE:
    case NULL_LINE:
      break;
    case MACRO_DEF:
      break;
    case MACRO_END:
      break;
    case MACRO_CALL:
      if (macro) { /*checking if the current macro is valid so we can append
                        line to the am file*/
        void *first_line = get_first_item((*macro)->lines);
        void *last_line = get_last_item((*macro)->lines);
        void *curr_line = first_line;
        while (&curr_line <= &last_line) {
          char *line = *(char **)curr_line;
          fputs(line, am_file);
          curr_line++;
        }
      }

      *macro = NULL; /*turn the flag off*/
      break;
    case REGULAR_LINE:
      if (macro) { /*inside a macro definition, add line to macro*/
        insert_item((*macro)->lines, linebuffer);
      }
      /*not inside a macro definition*/
      else {
        fputs(linebuffer, am_file);
      }
      break;
    }
  }

  /*deallocate memory from data structures and file pointers*/
  destroy_mcr(*macro);
  destroy_dynamic_array(macro_table);
  trie_destroy(mcr_search);
  /*free(as_file);*/
  fclose(as_file);
  fclose(am_file);
  free(as_file);

  return am_file_name;
}
