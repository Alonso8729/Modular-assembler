#include "lineToStructure.h"
#include <stdio.h>
#include <string.h>

int main() {
    char line_buffer[100] = {0};
    char save[100];
    syntax_tree st;
    while(fgets(line_buffer,sizeof(line_buffer),stdin)) {
        strcpy(save,line_buffer);
        st = get_tree_from_line(line_buffer);
        printf(":: %s\n",st.syntax_error);
    }
    return 0;
}