#include "assembler.h"

int second_pass(obj_file obj, FILE *am_file) {
  int binary_code;
  machine_word *m_word;
  int line_counter = 1;
  int i;
  int data_size;
  syntax_tree ast;
  char line_buffer[MAX_LINE] = {0};
  struct symbol *does_sym_exist = {0};
  char str[MAX_LINE];
  int comp_error_flag = 0;
  while (fgets(line_buffer, MAX_LINE, am_file)) {
    ast = get_tree_from_line(line_buffer);
    switch (ast.syntax_tree_options) {
    case syntax_tree_instruction:
      /*first we encode the first instruction word, which is common for all
       * instruction lines, using masking*/
      binary_code = ast.instruction_or_directive.syntax_tree_instruction
                        .syntax_tree_operand_options[1]
                    << 2; /*destination operand*/
      binary_code |= ast.instruction_or_directive.syntax_tree_instruction
                         .syntax_tree_operand_options[0]
                     << 9;
      binary_code |= ast.instruction_or_directive.syntax_tree_instruction
                         .syntax_tree_instruction_options
                     << 5;
      m_word = create_machine_word(binary_code);
      insert_item(obj->code_image, m_word);

      /*if both operands are register we combine them into one binary machine
     word*/
      if (ast.instruction_or_directive.syntax_tree_instruction
                  .syntax_tree_operand_options[0] == op_is_register &&
          ast.instruction_or_directive.syntax_tree_instruction
                  .syntax_tree_operand_options[1] == op_is_register) {
        binary_code = ast.instruction_or_directive.syntax_tree_instruction
                          .syntax_tree_instruction_operands[1]
                          .register_name
                      << 2;
        binary_code |= ast.instruction_or_directive.syntax_tree_instruction
                           .syntax_tree_instruction_operands[0]
                           .register_name
                       << 7;
        m_word = create_machine_word(binary_code);
        insert_item(obj->code_image, m_word);
      } else { /*for every other operands combination, we are going to loop the
                  operand's operands twice */
        for (i = 0; i < OP_MAX_NUM; i++) {
          switch (ast.instruction_or_directive.syntax_tree_instruction
                      .syntax_tree_operand_options[i]) {
          case op_is_register:
            binary_code = ast.instruction_or_directive.syntax_tree_instruction
                              .syntax_tree_instruction_operands[i]
                              .register_name
                          << 7;
            m_word = create_machine_word(binary_code);
            insert_item(obj->code_image, m_word);
            break;

          case op_is_const_num:
            binary_code = ast.instruction_or_directive.syntax_tree_instruction
                              .syntax_tree_instruction_operands[i]
                              .const_number
                          << 2;
            m_word = create_machine_word(binary_code);
            insert_item(obj->code_image, m_word);
            break;

          case op_is_label:
            does_sym_exist =
                find_str(obj->symbol_search->root,
                         ast.instruction_or_directive.syntax_tree_instruction
                             .syntax_tree_instruction_operands[i]
                             .label);
            if (does_sym_exist) {
              if (does_sym_exist->symbol_types == external) {
                binary_code = 1;
                m_word = create_machine_word(binary_code);
                insert_item(obj->code_image, m_word);
              } else if (does_sym_exist->symbol_types != entry) {
                binary_code = does_sym_exist->address << 2;
                binary_code |= 2; /*Relocatable*/
                m_word = create_machine_word(binary_code);
                insert_item(obj->code_image, m_word);
              }
            } else {
              /*PRINT ERROR The label %s is used but not defined*/
              printf("The label %s is used in line %d used but not defined\n",
                     ast.instruction_or_directive.syntax_tree_instruction
                         .syntax_tree_instruction_operands[i]
                         .label,
                     line_counter);

              comp_error_flag = 1;
            }
            break;

          case no_op:
            break;
          }
        }
      }
      break;
    case syntax_tree_directive:
      switch (ast.instruction_or_directive.syntax_tree_directive
                  .directive_options) {
      case directive_data:
        data_size = ast.instruction_or_directive.syntax_tree_directive
                        .directive_operand.data.data_count;
        for (i = 0; i < data_size; i++) {
          binary_code = ast.instruction_or_directive.syntax_tree_directive
                            .directive_operand.data.data[i];
          m_word = create_machine_word(binary_code);
          insert_item(obj->data_image, m_word);
        }
        break;

      case directive_string:
        strcpy(str, ast.instruction_or_directive.syntax_tree_directive
                        .directive_operand.string);
        int str_length=strlen(str);
        
        for(i=0; i<str_length; i++) { /*each character is a binary code word inserted to the
                         data section*/
          binary_code = str[i];
          m_word = create_machine_word(binary_code);
          insert_item(obj->data_image, m_word);
        }
        binary_code = 0;
        m_word = create_machine_word(binary_code);
        insert_item(obj->data_image, m_word);

        break;

      case directive_entry:
        break;

      case directive_extern:
        break;
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