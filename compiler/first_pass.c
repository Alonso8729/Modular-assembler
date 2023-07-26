#include "first_pass.h"
#include "assembler.h"
static void add_to_extern_table(obj_file *obj,short address,
                                char *extern_name) {
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
  extern_symbol* new_ext = create_extern_symbol(extern_name, address);
  new_ext->call_address = create_dynamic_array(sizeof(short));
  insert_item(new_ext->call_address, &address);
  insert_item(curr_obj->extern_table, &new_ext);
  return;
}

/*
static int check_defined_entry_labels(obj_file *obj) {
  obj_file curr_obj = *obj;

  int i;
}
*/
int first_pass(FILE *am_file, obj_file obj) {
  int line_counter = 1;
  struct symbol *does_sym_exist = {0};
  struct symbol tmp_symbol = {0};
  char line_buffer[MAX_LINE] = {0};
  Ast ast;
  char str[MAX_LINE];
  int data_count;
  unsigned short IC = 0, DC = 0; /*instructions counter, data counter*/
  int comp_error_flag = 0;
  int i;
  while (fgets(line_buffer, MAX_LINE, am_file)) {
    ast = lexer_get_ast(line_buffer);
    if (ast.ast_errors[0] != '\0') { /*check for syntax errors*/
      /*PRINT SYNTAX ERROR*/
      line_counter++;
      comp_error_flag = 1;
    }
    if (ast.label_name[0] != '\0') { /*check for label declaration*/
      strcpy(tmp_symbol.symbol_name, ast.label_name);
      does_sym_exist =
          find_str(obj->symbol_search->root, tmp_symbol.symbol_name);
      if (ast.ast_options == ast_directive) {
        if (does_sym_exist) { /*symbol exist in the symbol table*/
          if (does_sym_exist->symbol_types ==
              entry) { /*definition after entry declaration */
            does_sym_exist->symbol_types = entry_data;
            does_sym_exist->address = IC + DC + BASE_ADDRESS;
            does_sym_exist->declared_line = line_counter;
          } else {
            /*ERROR: "Label was already defined."*/
            comp_error_flag = 1;
          }
        } else { /*symbol was not found in the symbol table*/
          tmp_symbol.symbol_types = data;
          tmp_symbol.address = IC + DC + BASE_ADDRESS;
          tmp_symbol.declared_line = line_counter;
          insert_to_trie(tmp_symbol.symbol_name, obj->symbol_search->root,
                         insert_item(obj->symbol_table, &tmp_symbol));
        }
      } else if (ast.ast_options == ast_operation) {
        if (does_sym_exist) {
          if (does_sym_exist->symbol_types == entry) {
            does_sym_exist->symbol_types = entry_code;
            does_sym_exist->address = IC + BASE_ADDRESS;
            does_sym_exist->declared_line = line_counter;
          } else {
            /*ERROR: "Label was already defined."*/
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
    switch (ast.ast_options) {
    case ast_directive:
      if (ast.operation_and_directive.ast_directive.ast_directive_all ==
          ast_directive_data) {
        data_count = ast.operation_and_directive.ast_directive
                         .directive_operands.data.data_count;
        DC += data_count;
      } else if (ast.operation_and_directive.ast_directive.ast_directive_all ==
                 ast_directive_string) {
        strcpy(str, ast.operation_and_directive.ast_directive.directive_operands
                        .string);
        IC += strlen(str);
      } else { /*entry or extern*/
        does_sym_exist = find_str(obj->symbol_search->root,
                                  ast.operation_and_directive.ast_directive
                                      .directive_operands.label_name);
        if (does_sym_exist) {
          if (ast.operation_and_directive.ast_directive.ast_directive_all ==
              ast_directive_extern) {
            if (does_sym_exist->symbol_types == external) {
              /*PRINT WARNING REDEFINITION, label was already defined as extern
               */
            } else {
              /*PRINT ERROR, LABEL WAS ALREADY DEFINED IN THIS FILE AS EXTERN*/
              comp_error_flag = 1;
            }
          } else { /*entry directive*/
            if (does_sym_exist->symbol_types == external) {
              /*PRINT ERROR, Extern label can't be defined in the same file as
               * entry*/
              comp_error_flag = 1;
            } else if (does_sym_exist->symbol_types == entry ||
                       does_sym_exist->symbol_types == entry_code ||
                       does_sym_exist->symbol_types == entry_data)
              /*Print warning: label was already defined as entry*/
              printf("...");
            else {
              if (does_sym_exist->symbol_types == external)
                /*print redef warning*/
                printf("...");
              else {
                /*print redef error, defined as extern and now defined is this
                 * file again*/
                comp_error_flag = 1;
              }
            }
          }
        } else {
          strcpy(tmp_symbol.symbol_name,
                 ast.operation_and_directive.ast_directive.directive_operands
                     .label_name);
          tmp_symbol.symbol_types =
              ast.operation_and_directive.ast_directive.ast_directive_all ==
                      ast_directive_entry
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

    case ast_operation:
      IC++; /*incremeting IC for the operation's binary code*/
      /*if both operands are register we combine them into one binary machine
       * code*/
      if (ast.operation_and_directive.ast_operand_options[0] ==
              op_is_register &&
          ast.operation_and_directive.ast_operand_options[0] == op_is_register)
        IC++;
      else { /*every other combination of operands*/
        for (i = 0; i < OP_MAX_NUM; i++) {
          switch (ast.operation_and_directive.ast_operand_options[i]) {
          case op_is_label:
            IC++;
            strcpy(tmp_symbol.symbol_name,
                   ast.operation_and_directive.ast_directive.directive_operands
                       .label_name);
            does_sym_exist =
                find_str(obj->symbol_search->root, tmp_symbol.symbol_name);
            if (does_sym_exist && does_sym_exist->symbol_types == external)
              add_to_extern_table(&obj, IC + BASE_ADDRESS,
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
    case ast_empty_line:
      break;
    }
    line_counter++;
  }

  /*return obj file only if compilation succeeded*/
  return comp_error_flag == 0 ? 1 : 0;
}