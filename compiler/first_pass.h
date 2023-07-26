#ifndef __FIRST_PASS_H_
#define __FIRST_PASS_H_
#include "assembler.h"


/**
 * @brief 
 * 
 * @param am_file 
 * @param obj 
 * @return obj_file return 1 if compilation succeeded,otherwise return 0
 */
int first_pass(FILE *am_file, obj_file obj);


#endif