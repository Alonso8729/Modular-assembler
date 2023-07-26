#include "assembler.h"

int main(int argc,char** argv){
    return compile_program(argc-1, argv+1) ; /*ignoring the first input word*/
}