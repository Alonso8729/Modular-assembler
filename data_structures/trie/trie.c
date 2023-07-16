/**
 * @file trie.c
 * @brief Implementation of a trie data structure
 * @version 0.1
 * @date 2023-06-13
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "trie.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief helper function to deallocate all nodes memory in recurssion
 *
 * @param node
 */
static void trie_free_helper(trie_node node) {
  if (node == NULL)
    return;
  int i;
  for (i = 0; i < ASCII_CHARS_NUM; i++) {
    if (!node->child[i])
      trie_free_helper(node->child[i]);
  }
  free(node);
}

trie_node create_node() {
  trie_node node = calloc(1, sizeof(struct prefix_tree_node));
  node->end_of_str = NULL;
  return node;
}

Trie create_trie() {
  Trie t = calloc(1, sizeof(Trie));
  t->root = create_node();
  return t;
}

void *insert_to_trie(const char *str, trie_node root, void *end_of_word) {
  if (str == NULL)
    return NULL;
  int len = strlen(str);
  trie_node curr = root;
  int i;
  int index;
  for (i = 0; i < len; i++) {
    index = (str[i] - BASE_CHAR);
    if (curr->child[index] == NULL)
      curr->child[index] = create_node();
    curr = curr->child[index];
  }

  curr->end_of_str = end_of_word;
  return curr->end_of_str;
}

void *find_str(trie_node root, const char *str) {
  if (str == NULL)
    return NULL;
  int len = strlen(str);
  trie_node curr = root;
  int i;
  int index;
  for (i = 0; i < len; i++) {
    index = (str[i] - BASE_CHAR);
    if (curr->child[index] == NULL)
      return NULL;
    curr = curr->child[index];
  }
  return curr->end_of_str;
}

void trie_destroy(Trie t) {
  trie_free_helper(t->root);
  free(t);
}