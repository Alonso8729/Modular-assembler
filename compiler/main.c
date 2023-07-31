#include "assembler.h"

int main(int argc,char** argv){
    if (argc < 2) {
        printf( "Expected at least one assembly source file");
        exit(1);
    }
    return compile_program(argc-1, argv+1) ; /*ignoring the first input word*/
}