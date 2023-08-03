#include "first_pass.h"
#include "assembler.h"
#include <stdio.h>

static void add_to_extern_table(obj_file *obj, int address, char *extern_name) {
  int i;
  obj_file curr_obj = *obj;
  int table_size = get_item_count(curr_obj->extern_table);
  /*search for extern name in the extern table*/
  for (i = 0; i < table_size; i++) {
    struct extern_table *curr_ext;
    curr_ext = get_item(curr_obj->extern_table, i);
    if (strcmp(extern_name, curr_ext->extern_name) == 0) {
      insert_item(curr_ext->call_address, &address);
      return;
    }
  }
  /*adding a new item to the extern table*/
  extern_symbol *new_ext = create_extern_symbol(extern_name, address);
  new_ext->call_address = create_dynamic_array(sizeof(int));
  insert_item(new_ext->call_address, &address);
  insert_item(curr_obj->extern_table, new_ext);
  return;
}

int first_pass(FILE *am_file, obj_file obj) {
  int line_counter = 1;
  struct symbol *does_sym_exist = {0};
  struct symbol tmp_symbol = {0};
  char line_buffer[MAX_LINE] = {0};
  syntax_tree ast;
  char str[MAX_LINE];
  int data_count;
  int IC = 0, DC = 0; /*instructions counter, data counter*/
  int comp_error_flag = 0;
  int i;
  while (fgets(line_buffer, MAX_LINE, am_file)) {
    ast = get_tree_from_line(line_buffer);
    if (ast.syntax_error[0] != '\0') { /*check for syntax errors*/
      /*PRINT SYNTAX ERROR*/
      printf("Syntax error: %s\n", ast.syntax_error);
      line_counter++;
      comp_error_flag = 1;
      memset(line_buffer, 0, sizeof(line_buffer)); /*clean buffer*/
      continue;
    }
    if (ast.label_name[0] != '\0') { /*check for label declaration*/
      strcpy(tmp_symbol.symbol_name, ast.label_name);
      does_sym_exist =
          find_str(obj->symbol_search->root, tmp_symbol.symbol_name);
      if (ast.syntax_tree_options == syntax_tree_directive) {
        if (does_sym_exist) { /*Label exists in the symbol table*/
          if (does_sym_exist->symbol_types ==
              entry) { /*definition after entry declaration */
            does_sym_exist->symbol_types = entry_data;
            does_sym_exist->address = IC + DC + BASE_ADDRESS;
            does_sym_exist->declared_line = line_counter;
          } else {
            /*ERROR: "Label was already defined."*/
            printf("Label %s was already defind in line %d\n",
                   does_sym_exist->symbol_name, does_sym_exist->declared_line);
            comp_error_flag = 1;
          }
        } else { /*Label was not found in the symbol table*/
          tmp_symbol.symbol_types = data;
          tmp_symbol.address = IC + DC + BASE_ADDRESS;
          tmp_symbol.declared_line = line_counter;
          void *end_of_word;
          end_of_word = insert_item(obj->symbol_table, &tmp_symbol);
          insert_to_trie(tmp_symbol.symbol_name, obj->symbol_search->root,
                         end_of_word);
        }
      } else if (ast.syntax_tree_options == syntax_tree_instruction) {
        if (does_sym_exist) {
          if (does_sym_exist->symbol_types == entry) {
            does_sym_exist->symbol_types = entry_code;
            does_sym_exist->address = IC + BASE_ADDRESS;
            does_sym_exist->declared_line = line_counter;
          } else {
            /*ERROR: "Label was already defined."*/
            printf("REDEFINITION:Label %s was already defined as entry in "
                   "line %d\n",
                   does_sym_exist->symbol_name, does_sym_exist->declared_line);
            comp_error_flag = 1;
          }

        } else { /*symbol was not found in the symbol table*/
          tmp_symbol.symbol_types = code;
          tmp_symbol.address = IC + BASE_ADDRESS;
          tmp_symbol.declared_line = line_counter;
          insert_to_trie(tmp_symbol.symbol_name, obj->symbol_search->root,
                         insert_item(obj->symbol_table, &tmp_symbol));
        }
      }
    } /*end of symbol insertion*/
    /*check for addressing type and increment IC and DC accordingly*/
    switch (ast.syntax_tree_options) {
    case syntax_tree_directive:
      if (ast.instruction_or_directive.syntax_tree_directive
              .directive_options == directive_data) {
        data_count = ast.instruction_or_directive.syntax_tree_directive
                         .directive_operand.data.data_count;
        DC += data_count;
      } else if (ast.instruction_or_directive.syntax_tree_directive
                     .directive_options == directive_string) {
        strcpy(str, ast.instruction_or_directive.syntax_tree_directive
                        .directive_operand.string);
        IC += strlen(str);
      } else { /*entry or extern*/
        does_sym_exist =
            find_str(obj->symbol_search->root,
                     ast.instruction_or_directive.syntax_tree_directive
                         .directive_operand.label_name);
        if (does_sym_exist) {
          if (ast.instruction_or_directive.syntax_tree_directive
                  .directive_options == directive_extern) {
            if (does_sym_exist->symbol_types == external) {
              /*PRINT WARNING REDEFINITION, label was already defined as
               * extern*/
              printf("REDEFINITION:Label %s was already defined as entry in "
                     "line %d\n",
                     does_sym_exist->symbol_name,
                     does_sym_exist->declared_line);

            } else {
              /*PRINT ERROR, LABEL WAS ALREADY DEFINED IN THIS FILE AS EXTERN*/
              printf("Label %s was already defined as extern in line %d\n",
                     does_sym_exist->symbol_name,
                     does_sym_exist->declared_line);
              comp_error_flag = 1;
            }
          } else { /*entry directive*/
            if (does_sym_exist->symbol_types == external) {
              /*PRINT ERROR, Extern label can't be defined in the same file as
               * entry*/
              printf("Label %s was already defined as extern in line %d and "
                     "now defined as entry\n",
                     does_sym_exist->symbol_name,
                     does_sym_exist->declared_line);
              comp_error_flag = 1;
            } else if (does_sym_exist->symbol_types == entry ||
                       does_sym_exist->symbol_types == entry_code ||
                       does_sym_exist->symbol_types == entry_data)
              /*Print warning: label was already defined as entry*/
              printf("REDEFINITION:Label %s was already defined as entry in "
                     "line %d\n",
                     does_sym_exist->symbol_name,
                     does_sym_exist->declared_line);
            else {
              if (does_sym_exist->symbol_types == external)
                /*print redef warning*/
                printf("REDEFINITION:Label %s was already defined as extern in "
                       "line %d\n",
                       does_sym_exist->symbol_name,
                       does_sym_exist->declared_line);
              else {
                /*Label was define as data or code and now declared*/
                does_sym_exist->symbol_types =
                    does_sym_exist->symbol_types == code ? entry_code
                                                         : entry_data;
              }
            }
          }
        } else {
          strcpy(tmp_symbol.symbol_name,
                 ast.instruction_or_directive.syntax_tree_directive
                     .directive_operand.label_name);
          tmp_symbol.symbol_types =
              ast.instruction_or_directive.syntax_tree_directive
                          .directive_options == directive_entry
                  ? entry
                  : external; /*update */
          tmp_symbol.declared_line = line_counter;
          tmp_symbol.address = 0;
          /*need to update extern_call or entry call address*/
          insert_to_trie(tmp_symbol.symbol_name, obj->symbol_search->root,
                         insert_item(obj->symbol_table, &tmp_symbol));
        }
      }
      break;

    case syntax_tree_instruction:
      IC++; /*incremeting IC for the instruction's binary code*/
      /*if both operands are register we combine them into one binary machine
       * code*/
      if (ast.instruction_or_directive.syntax_tree_instruction
                  .syntax_tree_operand_options[0] == op_is_register &&
          ast.instruction_or_directive.syntax_tree_instruction
                  .syntax_tree_operand_options[1] == op_is_register)
        IC++;
      else { /*every other combination of operands*/
        for (i = 0; i < OP_MAX_NUM; i++) {
          switch (ast.instruction_or_directive.syntax_tree_instruction
                      .syntax_tree_operand_options[i]) {
          case op_is_label:
            IC++;
            strcpy(tmp_symbol.symbol_name,
                   ast.instruction_or_directive.syntax_tree_instruction
                       .syntax_tree_instruction_operands[i]
                       .label);
            does_sym_exist =
                find_str(obj->symbol_search->root, tmp_symbol.symbol_name);
            if (does_sym_exist && does_sym_exist->symbol_types == external)
              add_to_extern_table(&obj, IC + BASE_ADDRESS - 1,
                                  does_sym_exist->symbol_name);
            break;
          case op_is_register:
          case op_is_const_num:
            IC++; /*one machine word for each case*/
            break;
          case no_op:
            break;
          }
        }
      }
      break;

    default:
      break;
    }
    line_counter++;
    memset(line_buffer, 0, sizeof(line_buffer)); /*clean buffer*/
  }

  /*return obj file only if compilation succeeded*/
  return comp_error_flag == 0 ? 1 : 0;
}