#ifndef _DYNAMIC_ARRAY_H
#define _DYNAMIC_ARRAY_H
#include <stddef.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct dynamic_array {
  void *items;
  size_t capacity;
  size_t item_size;
  size_t item_count;
};

typedef struct dynamic_array *d_arr;

/**
 * @brief Create a dynamic array
 *
 * @param item_size
 * @return d_arr returns a pointer to the new dynamic array
 */
d_arr create_dynamic_array(size_t item_size);

/**
 * @brief
 *
 * @param arr
 * @param item
 * @return void*
 */
void *insert_item(d_arr arr, void *item);

/**
 * @brief count array items
 *
 * @param
 * @return size_t how many items are in the dynamic array
 */
size_t get_item_count(d_arr arr);

/**
 * @brief Get the first item pointer
 *
 * @param arr
 *@param i index of item
 * @return void* pointer to the first item
 */
void *get_item(d_arr arr, size_t i);

/**
 * @brief deallocate the items in the dynamic array and the array pointer
 *
 */
void destroy_dynamic_array(d_arr);

#endif
