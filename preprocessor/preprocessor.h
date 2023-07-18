#ifndef _PREPROCCESSOR_H
#define _PREPROCCESSOR_H
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
/**
 * @brief read input from .as file, spread macro if exist and paste it in .am file
 * 
 * @param file_name as file name
 * @return const char* return am file name
 */
const char* preprocess(const char* file_name);







#endif