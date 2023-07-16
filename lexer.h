#ifndef __LEXER_H_
#define __LEXER_H_

#define MAX_NUMBER_SIZE 100

struct lexer_ast {              /* for each line the lexer_ast returns an abstract syntax tree which represents a structure for a given line */
    char ast_errors[300];       /* error messages */
    enum {
        ast_operation,
        ast_directive
    }ast_options;
    union {
        struct {
            enum {
                ast_directive_extern,
                ast_directive_entry,
                ast_directive_string,
                ast_directive_data
            }ast_directive_all;
            union {
                char * label_name;
                char * string;
                struct {
                    int data[MAX_NUMBER_SIZE];
                    int data_count;
                }data;
            }directive_operands;
        }ast_directive;
        struct {
            enum {
                ast_op_mov,     /* gets 2 operands */
                ast_op_cmp,
                ast_op_add,
                ast_op_sub,
                ast_op_lea,

                ast_op_not,     /* gets 1 operands */
                ast_op_clr,
                ast_op_inc,
                ast_op_dec,
                ast_op_jmp,
                ast_op_bne,
                ast_op_red,
                ast_op_prn,
                ast_op_jsr,

                ast_op_rts,     /* gets 0 operands */
                ast_op_stop
            }ast_operation_all;
        }ast_operations;
        union {
            int instant_addressing; /* 1 */
            int direct_addressing;  /* 3 */
            char * direct_register_addressing;  /* 5 */
        }ast_operation_operands[2];
    }operation_and_directive;
};

typedef struct lexer_ast ast;

lexer_ast lexer_get_ast(char *line);

#endif