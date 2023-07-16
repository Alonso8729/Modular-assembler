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
  MACRO_REDEFINION,
  REGULAR_LINE
};

struct macro {
  char mcr_name[1 + MAX_MCR_NAME]; /*extra char for '/0'*/
  char **lines;
  int lines_counter;
};

typedef struct macro *Macro;

static void *create_macro(const char *m_name) {
  Macro mcr = malloc(sizeof(struct macro));
  if (!mcr) {
    printf("Memory allocation failed");
    return NULL;
  }
  if (m_name)
    strcpy(mcr->mcr_name, m_name);
  else
    mcr->mcr_name[0] = '\0';
  mcr->lines_counter = 0;
  mcr->lines=NULL;
  return mcr;
}

static void destroy_mcr(Macro m) {
  if (!m)
    return;
  Macro tmp = m;
  int line_count = m->lines_counter;
  int i;
  for (i = 0; i < line_count; i++) {
    free(m->lines[i]); /* Free the memory allocated for each line*/
  }
  free(m->lines); /* Free the memory allocated for the lines array */
  free(tmp);
}

static enum line_options line_detector(char *line, Macro *macro,
                                       Trie mcr_search, d_arr mcr_table,
                                       int *macro_flag) {
  Macro tmp;
  char *token = line; /*char pointer to help us analyze line structure*/
  SKIP_SPACE(token);
  if (token[0] == '\0' || token[0] == '\n')
    return NULL_LINE;
  if (token[0] == ';')
    return COMMENT_LINE;

  token = strstr(line, "endmcro");
  if (token) { /*finished macro definition*/
    line = token;
    return MACRO_END;
  }

  token = strstr(line, "mcro");
  if (token) {         /*getting inside macro defination*/
    token += 4;        /*token is out of the string "mcro"*/
    SKIP_SPACE(token); /*skip to the macro name first character*/
    /*find the end of the mcro_name*/
    line = token;
    token = strpbrk(line, SPACE_CHARS);
    /*eliminate white spaces from macro name*/
    if (token != NULL) {
      *token = '\0';
    }
    /*assining macro name*/
    const char *mcro_name = line;
    /*checking if already defined*/
    tmp = find_str(mcr_search->root, mcro_name);
    if (tmp) { /*macro already exist*/
      return MACRO_REDEFINION;
    }
    /**strcpy(new_mcr.mcr_name, mcro_name);**/
    /*inserting the new macro to the macro_table and macro lookup trie*/
    *macro = create_macro(mcro_name);
    insert_to_trie((*macro)->mcr_name, mcr_search->root,
                   insert_item(mcr_table, *macro));
    /**macro = insert_item(mcr_table, &new_mcr);
    insert_to_trie(new_mcr.mcr_name, mcr_search->root, (*macro));*/
    return MACRO_DEF;
  }

  token = &line[strlen(line) - 1];
  while (isspace(*token))
    token--;
  *(token + 1) = '\0';

  Macro is_mcr_exist = find_str(mcr_search->root, line);
  *(token + 1) = '\n';

  if (!is_mcr_exist) { /*check if macro*/
    return REGULAR_LINE;
  }
  return MACRO_CALL;
}

const char *preprocess(const char *input_file_name) {
  /*variable declaration*/
  Macro macro = NULL;
  Trie mcr_search = NULL;
  int mcr_flag = 0;
  d_arr macro_table = NULL;
  enum line_options line_type;
  char linebuffer[MAX_LINE] = {0};
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
  FILE *am_file = fopen(am_file_name, "w");
  if (!as_file || !am_file) {
    perror("Failed to open the .as file\n");
    free(am_file_name);
    free(as_file_name);
    return NULL;
  }

  /*creating data structures*/
  macro = create_macro(NULL);
  mcr_search = create_trie();
  if (!mcr_search) {
    printf("Failed to create trie\n");
    destroy_dynamic_array(macro_table);
    fclose(as_file);
    fclose(am_file);
    free(am_file_name);
    free(as_file_name);
    return NULL;
  }
  macro_table = create_dynamic_array(sizeof(Macro));
  if (!macro_table) {
    printf("Failed to create dynamic array\n");
    trie_destroy(mcr_search);
    fclose(as_file);
    fclose(am_file);
    free(am_file_name);
    free(as_file_name);
    return NULL;
  }

  while (fgets(linebuffer, sizeof(linebuffer), as_file)) {
    line_type =
        line_detector(linebuffer, &macro, mcr_search, macro_table, &mcr_flag);
    switch (line_type) {
    case COMMENT_LINE:
      break;
    case NULL_LINE:
      break;
    case MACRO_DEF:
      mcr_flag = 1;
      break;
    case MACRO_END:
      mcr_flag = 0;
      break;
    case MACRO_REDEFINION:
      printf("Macro has already been defined\n");
      break;
    case MACRO_CALL:
      if (!mcr_flag) { /*checking if the current macro is valid so we can append
                        line to the am file*/
        int i;
        for (i = 0; i < macro->lines_counter; i++) {
          fputs(macro->lines[i], am_file);
        }
      }

      break;
    case REGULAR_LINE:
      if (mcr_flag) { /*inside a macro definition, add line to macro*/
        char *new_line;
        new_line = malloc(strlen(linebuffer) + 1);
        strcpy(new_line, linebuffer);
        if (!new_line) {
          printf("Memory allocation failed\n");
          exit(0);
        }
        macro->lines =
            realloc(macro->lines, (macro->lines_counter + 1) * sizeof(char *));
        if (!macro->lines) {
          printf("Memory allocation failed\n");
          exit(0);
        }
        macro->lines[macro->lines_counter] = new_line;
        macro->lines_counter++;
      }
      /*not inside a macro definition*/
      else {
        fputs(linebuffer, am_file);
      }
      break;
    }
    memset(linebuffer, 0, sizeof(linebuffer));
  }

  /*deallocate memory from data structures and file pointers*/

  destroy_mcr(macro);
  free(as_file_name);
  destroy_dynamic_array(macro_table);
  trie_destroy(mcr_search);
  fclose(as_file);
  fclose(am_file);

  return am_file_name;
}
