#include "preprocessor.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: %s <input_file>\n", argv[0]);
    return 1;
  }

  const char *input_file_name = argv[1];
  const char *output_file_name = preprocess(input_file_name);

  if (output_file_name) {
    printf("Preprocessing completed successfully. Output file: %s\n",
           output_file_name);
    free((void *)output_file_name);
  } else {
    printf("Preprocessing failed.\n");
  }

  return 0;
}