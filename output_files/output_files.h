#ifndef __OUTPUT_FILES_H
#define __OUTPUT_FILES_H
#define ENT_FILE ".ent"
#define EXT_FILE ".ext"
#define OB_FILE ".ob"
#include "../data_structures/common_structs/common.h"
/**
 * @brief output both .ext, .ent and .ob files
 * 
 * @param obj 
 * @param file_name 
 */
void output_all_files(obj_file obj,const char* file_name);



#endif