/**
 * @file table.c
 * @brief dynamic generic array to store symbols, binary code and etc
 * @version 0.1
 * @date 2023-06-15
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "dynamic_array.h"

d_arr create_dynamic_array(int item_size) {
  d_arr da = malloc(sizeof(struct dynamic_array));
  if (!da)
    return NULL; /*memory allocation failed*/
  da->capacity = 32;
  da->item_size = item_size;
  da->item_count = 0;
  da->items = malloc(da->capacity * da->item_size);
  if (!da->items) { /*memory allocation failed*/
    free(da);
    return NULL;
  }
  return da;
}

void *insert_item(d_arr arr, void *item) {
  void *tmp;
  if (arr->item_count == arr->capacity) {
    arr->capacity = (arr->capacity) * 2;
    tmp = realloc(arr->items, arr->capacity * arr->item_size);
    if (!tmp) {           /*memory allocation failed*/
      arr->capacity /= 2; /*return to previous size*/
      return NULL;
    }
    arr->items = tmp;
  }
  /*using casting to be able to use aritimetics*/
  void *new_item = ((char *)arr->items) + (arr->item_count * arr->item_size);
  memcpy(new_item, item, arr->item_size);
  arr->item_count++;

  return new_item;
}

void *get_item(d_arr arr, int index) {
  if (index >= arr->item_count) {
    return NULL; /*invalid index*/
  }
  return (void *)((char *)arr->items + (index * arr->item_size));
}

int get_item_count(d_arr arr) { return arr->item_count; }

void destroy_dynamic_array(d_arr array) {
  if (array->item_count > 0){
    free(array->items);
  }  
  free(array);
}