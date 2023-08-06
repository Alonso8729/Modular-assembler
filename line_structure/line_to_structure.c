/* for each line the syntax_tree returns an abstract syntax tree which
 * represents a structure for a given line */

#include "line_to_structure.h"
#include "../data_structures/trie/trie.h"
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SKIP_SPACE(ptr)                                                        \
  while (*ptr && isspace(*ptr))                                                \
  ptr++
#define SPACE_CHARS "\t\f\r\n\v "
#define MAX_REG_VALUE 7
#define MIN_REG_VALUE 0
#define MAX_CONSTANT 511
#define MIN_CONSTANT -512

static int check_numeric(const char *input_str, char **final_ptr, long *val,
                         long lower_limit, long upper_limit);
static char check_operand(char *operand_txt, char **label, int *const_num,
                          int *reg_num);
static void trie_init(Trie *instruction_lookup, Trie *directive_lookup);
syntax_tree get_tree_from_line(char *logical_line);

enum label_err_options {
  Valid_Label,
  Begins_With_Non_Alpha,
  Contains_Non_Alphanumeric,
  Exceeds_Max_Length
};
static enum label_err_options check_label(const char *label) {
  /* checks if the label starts with an alphabetic character.
  then goes through each character of the label to ensure it is alphanumeric.
  if the label's length exceeds MAX_LABEL_LEN, it returns Exceeds_Max_Length.
  if none of these errors occur, it returns Valid_Label. */
  const char *start = label;
  if (!isalpha(*label))
    return Begins_With_Non_Alpha;
  while (*++label && isalnum(*label))
    ;
  if (*label != '\0')
    return Contains_Non_Alphanumeric;
  return (label - start > MAX_LABEL_LEN) ? Exceeds_Max_Length : Valid_Label;
}

static int check_numeric(const char *input_str, char **final_ptr, long *val,
                         long lower_limit, long upper_limit) {
  /*checks for overflow or underflow by comparing the result against the
   * provided limits. */
  char *temp_ptr;
  *val = strtol(input_str, &temp_ptr, 10);
  while (isspace(*temp_ptr))
    temp_ptr++;
  if (*temp_ptr != '\0' || errno == ERANGE || *val > upper_limit ||
      *val < lower_limit) {
    return *temp_ptr != '\0' ? 0 : errno == ERANGE ? -2 : -3;
  }
  if (final_ptr)
    *final_ptr = temp_ptr;
  return 0;
}

static char check_operand(char *operand_txt, char **label, int *const_num,
                          int *reg_num) {
  /*check_operand returns: I - immediate
                           L - lable
                           R - register
                           N - unknown
                           E - empty
                           F - constant number overflow
  when get_tree_from_line gets this, it display related errors if needed*/
  char *aux;
  long num;
  int result;
  while (*operand_txt && isspace(*operand_txt))
    operand_txt++;
  if (*operand_txt == '\0')
    return 'E';
  if (*operand_txt == '@') {
    if (*(operand_txt + 1) == 'r' &&
        !(*(operand_txt + 2) == '+' || *(operand_txt + 2) == '-')) {
      result = check_numeric(operand_txt + 2, NULL, &num, MIN_REG_VALUE,
                             MAX_REG_VALUE);
      if (!result && reg_num)
        *reg_num = (int)num;
      return result ? 'N' : 'R';
    }
    return 'N';
  }
  if (isalpha(*operand_txt)) {
    aux = operand_txt;
    while (*aux && !isspace(*aux))
      aux++;
    if (*aux != '\0') {
      *aux = '\0';
      aux++;
      while (*aux && isspace(*aux))
        aux++;
      if (*aux != '\0')
        return 'N';
    }
    if (check_label(operand_txt) != Valid_Label)
      return 'N';
    if (label)
      *label = operand_txt;
    return 'L';
  }
  result = check_numeric(operand_txt, NULL, &num, MIN_CONSTANT, MAX_CONSTANT);
  if (!result && const_num)
    *const_num = num;
  return result < -2 ? 'F' : result == 0 ? 'I' : 'N';
}

static struct directive_opt {
  const char *directive_name;
  int key;

} directive_opt[4] = {
    {"extern", directive_extern},
    {"entry", directive_entry},
    {"string", directive_string},
    {"data", directive_data},
};

static struct instruction_opt {
  const char *inst_name;
  int key;
  /*I - Immediate = 1, L - Label = 3, R - Registers = 5
  according to mentioned table in the course booklet, following are valid
  methods of addressing for each operand*/
  const char *src_operand_options;
  const char *dst_operand_options;
} instruction_opt[16] = {
    {"mov", syntax_tree_inst_mov, "ILR", "LR"},
    {"cmp", syntax_tree_inst_cmp, "ILR", "ILR"},
    {"add", syntax_tree_inst_add, "ILR", "LR"},
    {"sub", syntax_tree_inst_sub, "ILR", "LR"},
    {"lea", syntax_tree_inst_lea, "L", "LR"},

    {"not", syntax_tree_inst_not, NULL, "LR"},
    {"clr", syntax_tree_inst_clr, NULL, "LR"},
    {"inc", syntax_tree_inst_inc, NULL, "LR"},
    {"dec", syntax_tree_inst_dec, NULL, "LR"},
    {"jmp", syntax_tree_inst_jmp, NULL, "LR"},
    {"bne", syntax_tree_inst_bne, NULL, "LR"},
    {"red", syntax_tree_inst_red, NULL, "LR"},
    {"prn", syntax_tree_inst_prn, NULL, "ILR"},
    {"jsr", syntax_tree_inst_jsr, NULL, "LR"},

    {"rts", syntax_tree_inst_rts, NULL, NULL},
    {"stop", syntax_tree_inst_stop, NULL, NULL},
};

static void trie_init(Trie *instruction_lookup, Trie *directive_lookup) {
  /* initializes one tree for instructions and one for directives.*/
  int i;
  *instruction_lookup = create_trie();
  *directive_lookup = create_trie();

  for (i = 0; i < 16; i++) {
    insert_to_trie(instruction_opt[i].inst_name, (*instruction_lookup)->root,
                   &instruction_opt[i]);
  }
  for (i = 0; i < 4; i++) {
    insert_to_trie(directive_opt[i].directive_name, (*directive_lookup)->root,
                   &directive_opt[i]);
  }
}

static void clean_mem_trie(Trie instruction_lookup, Trie directive_lookup) {
  trie_destroy(directive_lookup);
  trie_destroy(instruction_lookup);
}

static void handle_error(syntax_tree *st, const char *msg, char *operand_string,
                         const char *inst_name) {
  /*writes an error message into the syntax tree's error string*/
  sprintf(st->syntax_error, msg, inst_name, operand_string);
}

static void assign_operand_option(syntax_tree *st, int index,
                                  char operand_opt) {
  /*assigns an operand type to a specific index in the syntax tree's array of
   * operand options.*/
  st->instruction_or_directive.syntax_tree_instruction
      .syntax_tree_operand_options[index] = operand_opt == 'I' ? op_is_const_num
                                            : operand_opt == 'R'
                                                ? op_is_register
                                                : op_is_label;
}

static void instruction_error_handler(syntax_tree *st, char *operands_string,
                                      struct instruction_opt *optI) {
  /* checks if the instruction expects one or two operands and validates the
  operands and assigns them to the syntax tree. generating relevant errors*/
  char operand_opt;
  char *sep = strchr(operands_string, ',');
  if (sep) {
    if (strchr(sep + 1, ',')) {
      handle_error(st, "Detected multiple ',' separators.", NULL, NULL);
      return;
    }
    if (optI->src_operand_options == NULL) {
      handle_error(
          st,
          "Instruction '%s' anticipates a single operand, but received two.",
          NULL, optI->inst_name);
      return;
    }
    *sep = '\0';

    operand_opt =
        check_operand(operands_string,
                      &st->instruction_or_directive.syntax_tree_instruction
                           .syntax_tree_instruction_operands[0]
                           .label,
                      &st->instruction_or_directive.syntax_tree_instruction
                           .syntax_tree_instruction_operands[0]
                           .const_number,
                      &st->instruction_or_directive.syntax_tree_instruction
                           .syntax_tree_instruction_operands[0]
                           .register_name);
    if (operand_opt == 'N' || operand_opt == 'F' || operand_opt == 'E') {
      handle_error(st, "Encountered unrecognized operand: '%s'",
                   operands_string, NULL);
      return;
    }
    if (strchr(optI->src_operand_options, operand_opt) == NULL) {
      handle_error(
          st, "instruction '%s' with source operand '%s' doesn't supported",
          operands_string, optI->inst_name);
      return;
    }
    assign_operand_option(st, 0, operand_opt);

    operands_string = sep + 1;

    operand_opt =
        check_operand(operands_string,
                      &st->instruction_or_directive.syntax_tree_instruction
                           .syntax_tree_instruction_operands[1]
                           .label,
                      &st->instruction_or_directive.syntax_tree_instruction
                           .syntax_tree_instruction_operands[1]
                           .const_number,
                      &st->instruction_or_directive.syntax_tree_instruction
                           .syntax_tree_instruction_operands[1]
                           .register_name);
    if (operand_opt == 'N' || operand_opt == 'F' || operand_opt == 'E') {
      handle_error(st, "Encountered unrecognized operand: '%s' for destination",
                   operands_string, NULL);
      return;
    }
    if (strchr(optI->dst_operand_options, operand_opt) == NULL) {
      handle_error(
          st,
          "instruction '%s' with destination operand '%s' doesn't supported",
          operands_string, optI->inst_name);
      return;
    }
    assign_operand_option(st, 1, operand_opt);
  } else {
    if (optI->src_operand_options != NULL) {
      handle_error(st, "Instruction '%s' expects a separator token: ','.", NULL,
                   optI->inst_name);
      return;
    }
    operand_opt =
        check_operand(operands_string,
                      &st->instruction_or_directive.syntax_tree_instruction
                           .syntax_tree_instruction_operands[1]
                           .label,
                      &st->instruction_or_directive.syntax_tree_instruction
                           .syntax_tree_instruction_operands[1]
                           .const_number,
                      &st->instruction_or_directive.syntax_tree_instruction
                           .syntax_tree_instruction_operands[1]
                           .register_name);
    if (operand_opt != 'E' && optI->dst_operand_options == NULL) {
      handle_error(st, "Instruction '%s' expects no operands.", NULL,
                   optI->inst_name);
      return;
    }
    if (operand_opt == 'E' || operand_opt == 'F') {
      handle_error(st, "Instruction '%s' expects one operand.", NULL,
                   optI->inst_name);
      return;
    }
    if (strchr(optI->dst_operand_options, operand_opt) == NULL) {
      handle_error(
          st,
          "instruction '%s' with destination operand '%s' doesn't supported",
          operands_string, optI->inst_name);
      return;
    }
    assign_operand_option(st, 1, operand_opt);
  }
}

static void check_directive(syntax_tree *st, char *operands_string,
                            struct directive_opt *optD) {
  /*checks the type of directive and validates its operand*/
  char *split, *split2;
  int curr_num;
  if (optD->key <= directive_entry) {
    if (check_operand(operands_string,
                      &st->instruction_or_directive.syntax_tree_directive
                           .directive_operand.label_name,
                      NULL, NULL) != 'L') {
      sprintf(st->syntax_error, "directive: '%s' with invalid operand: '%s'.",
              optD->directive_name, operands_string);
    }
  } else if (optD->key == directive_string) {
    split = strchr(operands_string, '"');
    if (!split || !(split2 = strrchr(split + 1, '"'))) {
      sprintf(st->syntax_error, "directive: '%s' has no %s '\"': '%s'.",
              optD->directive_name, split ? "closing" : "opening",
              operands_string);
      return;
    }
    split++;
    *split2 = '\0';
    split2++;
    SKIP_SPACE(split2);
    if (*split2 != '\0') {
      sprintf(st->syntax_error,
              "directive: '%s' has extra text after the string: '%s'.",
              optD->directive_name, split2);
    }
    st->instruction_or_directive.syntax_tree_directive.directive_operand
        .string = split;
  } else if (optD->key == directive_data) {
    int num_count = 0;
    do {
      split = strchr(operands_string, ',');
      if (split) {
        *split = '\0';
      }
      switch (check_operand(operands_string, NULL, &curr_num, NULL)) {
      case 'I':
        st->instruction_or_directive.syntax_tree_directive.directive_operand
            .data.data[num_count++] = curr_num;
        st->instruction_or_directive.syntax_tree_directive.directive_operand
            .data.data_count = num_count;
        break;
      case 'F':
        sprintf(st->syntax_error, "directive: '%s' : overflowed number: '%s'.",
                optD->directive_name, operands_string);
        return;
      case 'E':
        sprintf(st->syntax_error,
                "directive: '%s' : got empty string (no operands) but expected "
                "immediate number.",
                optD->directive_name);
        return;
      default:
        sprintf(st->syntax_error,
                "directive: '%s' : got non-number string: '%s'.",
                optD->directive_name, operands_string);
        return;
      }
      operands_string = split + 1;
    } while (split);
  }
}

syntax_tree get_tree_from_line(char *logical_line) {
  /*converts a logical line of code into a syntax tree.*/
  syntax_tree st = {0};
  enum label_err_options labelErr;
  struct instruction_opt *optI = NULL;
  struct directive_opt *optD = NULL;
  char *aux1, *aux2;
  Trie instruction_lookup;
  Trie directive_lookup;
  trie_init(&instruction_lookup, &directive_lookup);

  logical_line[strcspn(logical_line, "\r\n")] = 0;
  SKIP_SPACE(logical_line);

  if ((aux1 = strchr(logical_line, ':'))) {
    *aux1 = '\0';
    if ((aux2 = strchr(aux1 + 1, ':'))) {
      strcpy(st.syntax_error, "Duplicate ':' found in the line.");
      return st;
    }
    labelErr = check_label(logical_line);
    if (labelErr != Valid_Label) {
      if (labelErr == Begins_With_Non_Alpha)
        sprintf(st.syntax_error,
                "Label '%s' doesn't start with an alphabetic character.",
                logical_line);
      else if (labelErr == Contains_Non_Alphanumeric)
        sprintf(st.syntax_error,
                "Label '%s' contains non-alphanumeric characters.",
                logical_line);
      else
        sprintf(st.syntax_error,
                "Length of label '%s' exceeds the maximum limit of %d.",
                logical_line, MAX_LABEL_LEN);
      return st;
    }
    strcpy(st.label_name, logical_line);
    logical_line = aux1 + 1;
    SKIP_SPACE(logical_line);
  }

  if (*logical_line == '\0' && st.label_name[0] != '\0') {
    sprintf(st.syntax_error, "Line only has a label: '%s'.", st.label_name);
    return st;
  }

  aux1 = strpbrk(logical_line, SPACE_CHARS);
  if (aux1) {
    *aux1 = '\0';
    aux1++;
  } else {
    aux1 = logical_line;
  }
  SKIP_SPACE(aux1);

  if (*logical_line == '.') {
    optD = find_str(directive_lookup->root, logical_line + 1);
    if (optD) {
      st.syntax_tree_options = syntax_tree_directive;
      st.instruction_or_directive.syntax_tree_directive.directive_options =
          optD->key;
      check_directive(&st, aux1, optD);
    } else {
      sprintf(st.syntax_error, "Encountered an unrecognized directive: '%s'.",
              logical_line + 1);
      return st;
    }
  } else {
    optI = find_str(instruction_lookup->root, logical_line);
    if (optI) {
      st.syntax_tree_options = syntax_tree_instruction;
      st.instruction_or_directive.syntax_tree_instruction
          .syntax_tree_instruction_options = optI->key;
      if (optI->key < 14)
        instruction_error_handler(&st, aux1, optI);
    } else {
      sprintf(st.syntax_error, "Encountered unrecognized keyword: '%s'.",
              logical_line);
      return st;
    }
  }
  clean_mem_trie(instruction_lookup, directive_lookup);
  return st;
}