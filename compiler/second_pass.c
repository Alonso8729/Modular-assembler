#include "assembler.h"

int second_pass(obj_file obj, FILE *am_file) {
  short binary_code;
  machine_word *m_word;
  int i;
  int data_size;
  Ast ast;
  char line_buffer[MAX_LINE] = {0};
  struct symbol *does_sym_exist = {0};
  struct symbol tmp_symbol = {0};
  char str[MAX_LINE];
  int comp_error_flag = 0;
  while (fgets(line_buffer, MAX_LINE, am_file)) {
    ast = lexer_get_ast(line_buffer);
    switch (ast.ast_options) {
    case ast_operation:
      /*first we encode the first operation word, which is common for all
       * operation lines, using masking*/
      binary_code = ast.operation_and_directive.ast_operand_options[1]
                    << 2; /*destination operand*/
      binary_code |=
          ast.operation_and_directive.ast_operations.ast_operation_all << 5;
      binary_code |= ast.operation_and_directive.ast_operand_options[0] << 9;
      m_word = create_machine_word(binary_code);
      insert_item(obj->code_image, m_word);

      /*if both operands are register we combine them into one binary machine
     word*/
      if (ast.operation_and_directive.ast_operand_options[0] ==
              op_is_register &&
          ast.operation_and_directive.ast_operand_options[1] ==
              op_is_register) {
        binary_code =
            ast.operation_and_directive.ast_operation_operands[1].reg_num << 2;
        binary_code |=
            ast.operation_and_directive.ast_operation_operands[0].reg_num << 7;
        m_word = create_machine_word(binary_code);
        insert_item(obj->code_image, m_word);
      } else { /*for every other operands combination, we are going to loop the
                  operand's operands twice */
        for (i = 0; i < OP_MAX_NUM; i++) {
          switch (ast.operation_and_directive.ast_operand_options[i]) {
          case op_is_register:
            binary_code =
                ast.operation_and_directive.ast_operation_operands[i].reg_num
                << 7;
            m_word = create_machine_word(binary_code);
            insert_item(obj->code_image, m_word);
            break;

          case op_is_const_num:
            binary_code = ast.operation_and_directive.ast_operation_operands[i]
                              .constant_num
                          << 2;
            m_word = create_machine_word(binary_code);
            insert_item(obj->code_image, m_word);
            break;

          case op_is_label:
            does_sym_exist =
                find_str(obj->symbol_search->root,
                         ast.operation_and_directive.ast_operation_operands[i]
                             .label_name);
            if (does_sym_exist) {
              if (does_sym_exist->symbol_types == external) {
                binary_code = 1;
                m_word = create_machine_word(binary_code);
                insert_item(obj->code_image, m_word);
              } else if (does_sym_exist->symbol_types != entry) {
                binary_code = (unsigned short)does_sym_exist->address << 2;
                binary_code |= 2; /*Relocatable*/
                m_word = create_machine_word(binary_code);
                insert_item(obj->code_image, m_word);
              }
            } else {
              /*PRINT ERROR The label %s is used but not defined*/
              printf("The label %s is in line used but not defined\n",
                     ast.operation_and_directive.ast_operation_operands[i]
                         .label_name);

              comp_error_flag = 1;
            }
            break;

          case no_op:
            break;
          }
        }
      }
      break;
    case ast_directive:
      switch (ast.operation_and_directive.ast_directive.ast_directive_all) {
      case ast_directive_data:
        data_size = ast.operation_and_directive.ast_directive.directive_operands
                        .data.data_count;
        for (i = 0; i < data_size; i++) {
          binary_code = ast.operation_and_directive.ast_directive
                            .directive_operands.data.data[i];
          m_word = create_machine_word(binary_code);
          insert_item(obj->data_image, m_word);
        }
        break;

      case ast_directive_string:
        strcpy(str, ast.operation_and_directive.ast_directive.directive_operands
                        .string);
        while (*str) { /*each character is a binary code word inserted to the
                         data section*/
          binary_code = *str;
          m_word = create_machine_word(binary_code);
          insert_item(obj->data_image, m_word);
        }
        binary_code = 0;
        m_word = create_machine_word(binary_code);
        insert_item(obj->data_image, m_word);

        break;

      case ast_directive_entry:
        break;

      case ast_directive_extern:
        break;
      }
      break;

    case ast_empty_line:
      break;
    }
  }

  /*return obj file only if compilation succeeded*/
  return comp_error_flag == 0 ? 1 : 0;
}