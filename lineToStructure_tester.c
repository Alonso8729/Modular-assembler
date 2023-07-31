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
        printf("line buffer: %s , type of: %s\n",line_buffer,st.label_name);
    }
    return 0;
}