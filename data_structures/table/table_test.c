#include "stdio.h"
#include "table.h"
#include <stddef.h>
int main() {
    d_arr array = create_dynamic_array(sizeof(int));

    int value1 = 10;
    insert_item(array, &value1);

    int value2 = 20;
    insert_item(array, &value2);

    int value3 = 30;
    insert_item(array, &value3);

    int* first_item = get_first_item(array);
    if (first_item != NULL) {
        printf("First item: %d\n", *first_item);
    }

    int* last_item = get_last_item(array);
    if (last_item != NULL) {
        printf("Last item: %d\n", *last_item);
    }

    size_t count = get_item_count(array);
    printf("Item count: %lu\n", (unsigned long)count);

    destroy_dynamic_array(array);

    return 0;
}