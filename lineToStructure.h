#ifndef LINE_TO_STRUCTURE
#define LINE_TO_STRUCTURE

#define MAX_NUMBER_DATA 80
#define MAX_LABEL_LEN 31        /*+1 for the '\0' char*/

struct syntax_tree {              /* for each line the syntax_tree returns an abstract syntax tree which represents a structure for a given line */
    char label_name[MAX_LABEL_LEN+1];
    char syntax_error[150];
    enum {                      /*tell us whether this is an instruction or directive*/
        syntax_tree_instruction,
        syntax_tree_directive 
    }syntax_tree_options;
    union {
        struct {
            enum {
                directive_extern,
                directive_entry,
                directive_string,
                directive_data
            }directive_options;
            union {
                char * label_name;      /*for .entry or .extern*/
                char * string;          /*for .string*/
                struct {                /*for .data*/
                    int data[MAX_NUMBER_DATA];
                    int data_count;
                }data;
            }directive_operand;
        }syntax_tree_directive;
        struct {
                enum {
                    syntax_tree_inst_mov,     /* gets 2 operands */
                    syntax_tree_inst_cmp,
                    syntax_tree_inst_add,
                    syntax_tree_inst_sub,
                    syntax_tree_inst_lea,

                    syntax_tree_inst_not,     /* gets 1 operands */
                    syntax_tree_inst_clr,
                    syntax_tree_inst_inc,
                    syntax_tree_inst_dec,
                    syntax_tree_inst_jmp,
                    syntax_tree_inst_bne,
                    syntax_tree_inst_red,
                    syntax_tree_inst_prn,
                    syntax_tree_inst_jsr,

                    syntax_tree_inst_rts,     /* gets 0 operands */
                    syntax_tree_inst_stop
                }syntax_tree_instruction_options;
                enum {
                    no_op = 0,
                    op_is_const_num = 1,
                    op_is_register = 5,
                    op_is_label = 3
                }syntax_tree_operand_options[2];
                union {    /*total of 3 options for each operand*/
                    int const_number;
                    int register_name;  
                    char * label; 
                }syntax_tree_instruction_operands[2];
        }syntax_tree_instruction;
    }instruction_or_directive;
};
typedef struct syntax_tree syntax_tree;

syntax_tree get_tree_from_line(char *logical_line);

#endif