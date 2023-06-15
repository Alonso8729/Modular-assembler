#include "stdio.h"
#include "table.h"
#include <stddef.h>
int main() {
  d_arr symbolArray = create_dynamic_array(sizeof(char *));
  if (!symbolArray) {
    printf("Failed to create dynamic array for symbols.\n");
    return 1;
  }

  const char *symbol1 = "MAIN";
  const char *symbol2 = "LOOP";
  const char *symbol3 = "END";
  if (!insert_item(symbolArray, (void *)&symbol1)) {
    printf("Failed to insert symbol 1.\n");
    destroy_dynamic_array(symbolArray);
    return 1;
  }
  if (!insert_item(symbolArray, (void *)&symbol2)) {
    printf("Failed to insert symbol 2.\n");
    destroy_dynamic_array(symbolArray);
    return 1;
  }
  if (!insert_item(symbolArray, (void *)&symbol3)) {
    printf("Failed to insert symbol 3.\n");
    destroy_dynamic_array(symbolArray);
    return 1;
  }

  size_t symbolCount = get_item_count(symbolArray);
  printf("Symbols:\n");
  size_t i;
  for (i = 0; i < symbolCount; i++) {
    const char *symbol = *(const char **)(((char *)symbolArray->items) +
                                          (i * symbolArray->item_size));
    printf("%s\n", symbol);
  }

  destroy_dynamic_array(symbolArray);

  return 0;
}