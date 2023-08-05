
/* for each line the syntax_tree returns an abstract syntax tree which represents a structure for a given line */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include "lineToStructure.h"
#include "../data_structures/trie/trie.h"

#define SKIP_SPACE(ptr) while(*ptr && isspace(*ptr)) ptr++
#define SPACE_CHARS "\t\f\r\n\v "
#define MAX_REG_VALUE 7
#define MIN_REG_VALUE 0
#define MAX_CONSTANT 511
#define MIN_CONSTANT -512

static int check_numeric(const char *input_str, char **final_ptr, long * val, long lower_limit, long upper_limit);
static char check_operand(char * operand_txt, char ** label, int * const_num, int * reg_num);
static void trie_init();
syntax_tree get_tree_from_line(char *logical_line);

Trie instruction_lookup = NULL;
Trie directive_lookup = NULL;
static int is_trie_inited = 0;

enum label_err_options {
    Valid_Label, 
    Begins_With_Non_Alpha,
    Contains_Non_Alphanumeric,
    Exceeds_Max_Length
};
static enum label_err_options check_label(const char * label) {
    /* checks if the label it is alphanumeric and the label's length  */
    const char *start = label;
    if (!isalpha(*label)) return Begins_With_Non_Alpha;
    while (*++label && isalnum(*label));
    if (*label != '\0') return Contains_Non_Alphanumeric;
    return (label - start > MAX_LABEL_LEN) ? Exceeds_Max_Length : Valid_Label;
}

static int check_numeric(const char *input_str, char **final_ptr, long * val, long lower_limit, long upper_limit) {
    /*checks for overflow or underflow by comparing the result against the provided limits. */
    char *temp_ptr;
    *val = strtol(input_str, &temp_ptr, 10);
    while(isspace(*temp_ptr)) temp_ptr++;
    if (*temp_ptr != '\0' || errno == ERANGE || *val > upper_limit || *val < lower_limit) {
        return *temp_ptr != '\0' ? 0 : errno == ERANGE ? -2 : -3;
    }
    if (final_ptr) *final_ptr = temp_ptr;
    return 0;
}

static char check_operand(char * operand_txt, char ** label, int * const_num, int * reg_num) {
    /*check_operand returns: I - immediate 
                             L - lable
                             R - register
                             N - unknown
                             E - empty
                             F - constant number overflow
    wehere get_tree_from_line gets this, it display related errors if needed*/
    char * aux;
    long num;
    int result;
    while (*operand_txt && isspace(*operand_txt)) operand_txt++;
    if (*operand_txt == '\0') return 'E';
    if (*operand_txt == '@') {
        if (*(operand_txt + 1) == 'r' && !(*(operand_txt + 2) == '+' || *(operand_txt + 2) == '-')) {
            result = check_numeric(operand_txt + 2, NULL, &num, MIN_REG_VALUE, MAX_REG_VALUE);
            if (!result && reg_num) *reg_num = (int)num;
            return result ? 'N' : 'R';
        }
        return 'N';
    }
    if (isalpha(*operand_txt)) {
        aux = operand_txt;
        while(*aux && !isspace(*aux)) aux++;
        if(*aux != '\0') {
            *aux = '\0';
            aux++;
            while (*aux && isspace(*aux)) aux++;
            if (*aux != '\0') return 'N';
        }
        if (check_label(operand_txt) != Valid_Label) return 'N';
        if (label) *label = operand_txt;
        return 'L';
    }
    result = check_numeric(operand_txt, NULL, &num, MIN_CONSTANT, MAX_CONSTANT);
    if (!result && const_num) *const_num = num;
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
    according to mentioned table in the course booklet, following are valid methods of addressing for each operand*/
    const char *src_operand_options; 
    const char *dst_operand_options;
} instruction_opt[16] = {
    {"mov",syntax_tree_inst_mov, "ILR", "LR"},
    {"cmp",syntax_tree_inst_cmp, "ILR", "ILR"},
    {"add",syntax_tree_inst_add, "ILR", "LR"},
    {"sub",syntax_tree_inst_sub, "ILR", "LR"},
    {"lea",syntax_tree_inst_lea, "L", "LR"},

    {"not",syntax_tree_inst_not, NULL, "LR"},
    {"clr",syntax_tree_inst_clr, NULL, "LR"},
    {"inc",syntax_tree_inst_inc, NULL, "LR"},
    {"dec",syntax_tree_inst_dec, NULL, "LR"},
    {"jmp",syntax_tree_inst_jmp, NULL, "LR"},
    {"bne",syntax_tree_inst_bne, NULL, "LR"},
    {"red",syntax_tree_inst_red, NULL, "LR"},
    {"prn",syntax_tree_inst_prn, NULL, "ILR"},
    {"jsr",syntax_tree_inst_jsr, NULL, "LR"},

    {"rts",syntax_tree_inst_rts, NULL, NULL},
    {"stop",syntax_tree_inst_stop, NULL, NULL},
};

static void trie_init() {
    /* initializes one tree for instructions and one for directives.*/
    int i;
    instruction_lookup = create_trie();
    directive_lookup = create_trie();

    for(i=0;i<16;i++) {
        insert_to_trie(instruction_opt[i].inst_name,instruction_lookup->root, &instruction_opt[i]);
    }
    for(i=0;i<4;i++) {
        insert_to_trie(directive_opt[i].directive_name,directive_lookup->root, &directive_opt[i]);
    }
    is_trie_inited = 1;
}

static void clean_mem_trie(Trie instruction_lookup, Trie directive_lookup) {
    is_trie_inited = 0;
    trie_destroy(directive_lookup);
    trie_destroy(instruction_lookup);
}

static void instruction_error_handler(syntax_tree * st, char * operands_string, struct instruction_opt * optI) {
    /* checks if the instruction expects one or two operands and validates the operands and assigns them to the syntax tree.
    generating relevant errors*/
    char operand_opt;
    char * aux1;
    char * sep = strchr(operands_string, ',');
    if(sep) {
        aux1 = strchr(sep+1,',');
        if(aux1) {
            strcpy(st->syntax_error,"Detected multiple ',' separators.");
            return;
        }
        if(optI->src_operand_options == NULL) {
            sprintf(st->syntax_error,"Instruction '%s' anticipates a single operand, but received two.",optI->inst_name);
            return;
        }
        *sep = '\0';
        
        operand_opt = check_operand(operands_string,&st->instruction_or_directive.syntax_tree_instruction.syntax_tree_instruction_operands[0].label,
            &st->instruction_or_directive.syntax_tree_instruction.syntax_tree_instruction_operands[0].const_number,
             &st->instruction_or_directive.syntax_tree_instruction.syntax_tree_instruction_operands[0].register_name);
        if (operand_opt == 'N') {
            sprintf(st->syntax_error,"Encountered unrecognized operand: '%s'", operands_string);
            return; 
        }
        if(operand_opt == 'F') {
            sprintf(st->syntax_error,"overflow immediate operand: '%s' for source", operands_string);
            return;
        }
        if(operand_opt == 'E') {
            sprintf(st->syntax_error,"source got no operand");
            return;
        }
        if(strchr(optI->src_operand_options,operand_opt) == NULL) {
            sprintf(st->syntax_error,"instruction '%s' with non valid source operand '%s'",optI->inst_name,operands_string);
            return;
        }
        operands_string = sep+1;  
        operand_opt = check_operand(operands_string, &st->instruction_or_directive.syntax_tree_instruction.syntax_tree_instruction_operands[1].label,
                                                    &st->instruction_or_directive.syntax_tree_instruction.syntax_tree_instruction_operands[1].const_number,
                                                     &st->instruction_or_directive.syntax_tree_instruction.syntax_tree_instruction_operands[1].register_name);
        if (operand_opt == 'N') {
            sprintf(st->syntax_error,"Encountered unrecognized operand: '%s' for destination", operands_string);
            return; 
        }
        if(operand_opt == 'F') {
            sprintf(st->syntax_error,"immediate operand: '%s' for destination overflow", operands_string);
            return;
        }
        if(operand_opt == 'E') {
            sprintf(st->syntax_error,"destination got no operand");
            
            return;
        }
        if(strchr(optI->dst_operand_options,operand_opt) == NULL) {
            sprintf(st->syntax_error,"instruction: '%s' with non valid destination operand: '%s' ",optI->inst_name,operands_string);
            return;
        }
    }
    else {
        if(optI->src_operand_options != NULL) {
            sprintf(st->syntax_error,"instruction: '%s' expects seperator token: ','.",optI->inst_name);
            return;
        }
        operand_opt = check_operand(operands_string, &st->instruction_or_directive.syntax_tree_instruction.syntax_tree_instruction_operands[1].label,
                            &st->instruction_or_directive.syntax_tree_instruction.syntax_tree_instruction_operands[1].const_number,
                            &st->instruction_or_directive.syntax_tree_instruction.syntax_tree_instruction_operands[1].register_name);
        if(operand_opt == 'E') {
            SKIP_SPACE(operands_string);
            if (*operands_string != '\0') {
                sprintf(st->syntax_error, "instruction: '%s' expects no operands, but found '%s'.", optI->inst_name, operands_string);
                return;
            }
            sprintf(st->syntax_error, "instruction: '%s' expects one operand.", optI->inst_name);
            return;            
        }

        if(operand_opt == 'F') {
            sprintf(st->syntax_error,"immediate operand: '%s' for destination overflow", operands_string);
            return;
        }
        if(optI->dst_operand_options != NULL && strchr(optI->dst_operand_options,operand_opt) == NULL) {
            sprintf(st->syntax_error,"instruction: '%s' destination operand: '%s' is not supported",optI->inst_name,operands_string);
            return;
        }

    }
}

static void check_directive(syntax_tree *st, char *operands_string, struct directive_opt * optD) {
    /*checks the type of directive and validates its operand*/
    char * split;
    char * split2;
    int num_count = 0;
    int curr_num;
    if(optD->key <= directive_entry) {   /*entry or extern*/
        if(check_operand(operands_string,&st->instruction_or_directive.syntax_tree_directive.directive_operand.label_name,NULL,NULL) != 'L') {
            sprintf(st->syntax_error,"directive: '%s' with invalid operand: '%s'.", optD->directive_name,operands_string);
            return;
        }
    }
    else if(optD->key == syntax_tree_directive) {
        split = strchr(operands_string,'"');
        if(!split) {
            sprintf(st->syntax_error,"directive: '%s' has no openning '\"': '%s'.",optD->directive_name,operands_string);
        }
        split++;
        split2 = strrchr(split,'"');
        if(!split2) {
            sprintf(st->syntax_error,"directive: '%s' has no closing '\"': '%s'.",optD->directive_name,operands_string);
        }
        *split2 = '\0';
        split2++;
        SKIP_SPACE(split2);
        if(*split2 != '\0') {
            sprintf(st->syntax_error,"directive: '%s' has extra text after the string: '%s'.",optD->directive_name,split2);
        }
        st->instruction_or_directive.syntax_tree_directive.directive_operand.string = split;
    }
    else if(optD->key == directive_data) { /*data*/
        do {
            split = strchr(operands_string,',');
            if(split) { 
                *split = '\0';
            }
            switch(check_operand(operands_string,NULL,&curr_num,NULL)) {
                case 'I':
                    st->instruction_or_directive.syntax_tree_directive.directive_operand.data.data[num_count] = curr_num;
                    num_count++;
                    st->instruction_or_directive.syntax_tree_directive.directive_operand.data.data_count = num_count;
                break;
                case 'F':
                    sprintf(st->syntax_error,"directive: '%s' : overflowed number: '%s'.",optD->directive_name,operands_string);
                    return;
                break;
                case 'E':
                    sprintf(st->syntax_error,"directive: '%s' : got empty string (no operands) but expected immediate number.",optD->directive_name);
                    return;
                break;
                default:
                    sprintf(st->syntax_error,"directive: '%s' : got none number string: '%s'.",optD->directive_name,operands_string);
                    return;
                break;
            }
            if(split)
                operands_string = split+1;
            else {
                break;
            }
        } while(1);
    }
}

syntax_tree get_tree_from_line(char *logical_line) {
    /*converts a logical line of code into a syntax tree.*/
    syntax_tree st = {0};
    enum label_err_options labelErr;
    struct instruction_opt * optI = NULL; /*pointers for I - instructinos*/
    struct directive_opt  * optD = NULL; /*pointers for D - directives*/
    char *aux1, *aux2;
    if(!is_trie_inited) {
        trie_init();
        if (instruction_lookup == NULL || directive_lookup == NULL) {
            strcpy(st.syntax_error, "Failed to initialize the trie.");
            return st;
        }
    }
    logical_line[strcspn(logical_line, "\r\n")] = 0;
    SKIP_SPACE(logical_line);
    aux1 = strchr(logical_line, ':');

    if(aux1) {
        aux2 = strchr(aux1+1, ':');
        if (aux2) {
            strcpy(st.syntax_error, "Duplicate ':' found in the line.");
            return st;
        }
        (*aux1) = '\0';
        switch(labelErr = check_label(logical_line)) {
            case Begins_With_Non_Alpha:
                sprintf(st.syntax_error,"Label '%s' doesn't start with an alphabetic character.",logical_line);
            break;
            case Contains_Non_Alphanumeric:
                sprintf(st.syntax_error,"Label '%s' contains non-alphanumeric characters.",logical_line);
            break;
            case Exceeds_Max_Length:
                sprintf(st.syntax_error, "Length of label '%s' exceeds the maximum limit of %d.",logical_line,MAX_LABEL_LEN);
            break;
            case Valid_Label:
                strcpy(st.label_name,logical_line);
            break;
        }
        if(labelErr != Valid_Label) {
            return st;
        }
        logical_line = aux1+1;
        SKIP_SPACE(logical_line);
    }

    if(*logical_line == '\0' && st.label_name[0] != '\0') {
        sprintf(st.syntax_error, "Line only has a label: '%s'.",st.label_name);
        return st;
    }
    aux1 = strpbrk(logical_line,SPACE_CHARS); /*0x7fffffffd9c5 ".string “abcdef”"*/ 
    if(aux1)  { /*0x7fffffffd9cc " “abcdef”"*/
        (*aux1) = '\0';
        aux1++;
        }
    else {
        aux1 = logical_line;
    }
    SKIP_SPACE(aux1);
    if (*logical_line == '.') {
        optD = find_str(directive_lookup->root, logical_line + 1);
        if (optD) {
            st.syntax_tree_options = syntax_tree_directive;
            st.instruction_or_directive.syntax_tree_directive.directive_options = optD->key;
            check_directive(&st, aux1, optD);
        } else {
            sprintf(st.syntax_error, "Encountered an unrecognized directive: '%s'.", logical_line + 1);
            return st; 
            }
    } else {
        optI = find_str(instruction_lookup->root, logical_line); 
        if (optI) {
            st.syntax_tree_options = syntax_tree_instruction;
            st.instruction_or_directive.syntax_tree_instruction.syntax_tree_instruction_options = optI->key;
            instruction_error_handler(&st, aux1, optI);
    } else {
        sprintf(st.syntax_error, "Encountered unrecognized keyword: '%s'.", logical_line);
        return st; 
        }
    }
    return st;
}