#include "assembler.h"
#include <stdio.h>

static int start_compilation(const char *file_name, FILE *am_file,
                             obj_file obj) {

  d_arr missing_symbol_table =
      create_dynamic_array(sizeof(struct missing_sym_table));
  int line_counter = 1;
  char *str;
  int data_size;
  unsigned short binary_word = 0;
  int comp_error_flag = 1;
  unsigned short *binary_word_ptr;
  struct symbol tmp_sym = {0};
  struct missing_sym_table missing_sym = {0};
  struct symbol *does_sym_exist = {0};
  char line_buffer[MAX_LINE] = {0};
  int i;
  Ast ast;
  while (fgets(line_buffer, MAX_LINE, am_file)) {
    ast = lexer_get_ast(line_buffer);
    if (ast.ast_errors[0] != '\0') { /*check for syntax errors*/
      /*print syntax error*/
      continue;
    }
    if (ast.label_name[0] !=
        '\0') { /*check for label, insert to symbol table if needed*/
      strcpy(tmp_sym.symbol_name, ast.label_name);
      /*search for label in symbol table*/
      does_sym_exist = find_str(obj->symbol_search->root, tmp_sym.symbol_name);
      if (ast.ast_options == ast_directive) {
        if (does_sym_exist) {
          if (does_sym_exist->symbol_types ==
              entry) { /*defenition after entry declaration*/
            does_sym_exist->symbol_types = entry_data;
            does_sym_exist->address =
                (unsigned short)get_item_count(obj->data_section);
            does_sym_exist->declared_line = line_counter;
          } else { /*symbol type isn't entry, redefinition*/
            /*PRINT REDEFINITION ERROR*/
            comp_error_flag = 0;
          }
        } else { /*symbol wasn't found*/
          tmp_sym.symbol_types = data;
          tmp_sym.address = (unsigned short)get_item_count(obj->data_section);
          tmp_sym.declared_line = line_counter;
          insert_to_trie(tmp_sym.symbol_name, obj->symbol_search->root,
                         insert_item(obj->symbol_table, &tmp_sym));
        }
      } else if (ast.ast_options == ast_operation) {
        if (does_sym_exist) {
          if (does_sym_exist->symbol_types == entry) {
            does_sym_exist->symbol_types = entry_code;
            does_sym_exist->address = does_sym_exist->address =
                (unsigned short)get_item_count(obj->code_section) +
                BASE_ADDRESS;
          } else { /*symbol is not entry*/
            /*PRINT REDEF ERROR*/
            comp_error_flag = 0;
          }
        } else { /*symbol wasn't found*/
          tmp_sym.symbol_types = code;
          tmp_sym.address =
              (unsigned short)get_item_count(obj->code_section) + BASE_ADDRESS;
          tmp_sym.declared_line = line_counter;
          insert_to_trie(tmp_sym.symbol_name, obj->symbol_search->root,
                         insert_item(obj->symbol_table, &tmp_sym));
        }
      }
    } /*end of symbol insertion into symbol table*/
    /*beggining of binary machine words coding*/
    switch (ast.ast_options) {
    case ast_directive:
      switch (ast.operation_and_directive.ast_directive.ast_directive_all) {
      case ast_directive_string:
        str =
            ast.operation_and_directive.ast_directive.directive_operands.string;
        while (str) { /*each character is a binary code word inserted to the
                         data section*/
          binary_word = *str;
          insert_item(obj->data_section, &binary_word);
        }
        binary_word = 0;
        insert_item(obj->data_section, &binary_word);

        break;
      case ast_directive_data:
        data_size = ast.operation_and_directive.ast_directive.directive_operands
                        .data.data_count;
        for (i = 0; i < data_size; i++) {
          binary_word = ast.operation_and_directive.ast_directive
                            .directive_operands.data.data[i];
          insert_item(obj->data_section, &binary_word);
        }
        break;
      case ast_directive_extern:
      case ast_directive_entry:
        does_sym_exist = find_str(obj->symbol_search->root,
                                  ast.operation_and_directive.ast_directive
                                      .directive_operands.label_name);
        if (does_sym_exist) {
          if (ast.operation_and_directive.ast_directive.ast_directive_all ==
              ast_directive_entry) {
            if (does_sym_exist->symbol_types == entry ||
                does_sym_exist->symbol_types == entry_code ||
                does_sym_exist->symbol_types == entry_data)
              /*Print warning of entry REDEF*/
              printf("...");
            else if (does_sym_exist->symbol_types == external)
              /*Print error, defined as external, can't be entry*/
              comp_error_flag = 0;
            else {
              if (does_sym_exist->symbol_types == code)
                does_sym_exist->symbol_types = entry_code;
              does_sym_exist->symbol_types = entry_data;
            }
          } else {
            if (does_sym_exist->symbol_types == external)
              /*print redef warning*/
              printf("...");
            else {
              /*print redef error, defined as extern and now defined is this
               * file again*/
              comp_error_flag = 0;
            }
          }

        } else {
          strcpy(tmp_sym.symbol_name, ast.operation_and_directive.ast_directive
                                          .directive_operands.label_name);
          tmp_sym.symbol_types =
              (ast.operation_and_directive.ast_directive.ast_directive_all ==
               ast_directive_entry)
                  ? entry
                  : external;
          tmp_sym.address = 0;
          tmp_sym.declared_line = line_counter;
          insert_to_trie(tmp_sym.symbol_name, obj->symbol_search->root,
                         insert_item(obj->symbol_table, &tmp_sym));
        }

        break;
      }
      break; /*End of directive decoding*/

    case ast_operation:
      /*First, we decode the first operaion binary machine word, which is common
       * for all the operations lines, using masking*/
      binary_word = ast.operation_and_directive.ast_operand_options[1]
                    << 2; /*destination operand*/
      binary_word |=
          ast.operation_and_directive.ast_operations.ast_operation_all
          << 5; /*operation name*/
      binary_word |= ast.operation_and_directive.ast_operand_options[0]
                     << 9; /*source operand*/
      insert_item(obj->code_section, &binary_word);
      /*if both of the operands are registers, we combine them into one binary
       * word*/
      if (ast.operation_and_directive.ast_operand_options[0] ==
              op_is_register &&
          ast.operation_and_directive.ast_operand_options[1] ==
              op_is_register) {
                
      }
      else{ /*for every other operands combination, we are going to loop two times */

      }

      break;

    case ast_empty_line:
      break;
    }
    line_counter++;
  }
  return comp_error_flag;
}