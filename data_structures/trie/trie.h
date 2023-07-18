/**
 * @brief API implementation for a prefix tree for inserting strings and
 * searching strings efficiently
 */

#ifndef __TRIE_H_
#define __TRIE_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BASE_CHAR ' ' /*first usable char in the ASCII TABLE*/
#define ASCII_CHARS_NUM 95

struct prefix_tree_node {
  struct prefix_tree_node
      *child[ASCII_CHARS_NUM]; /*each pointer points to a char we are going
                   to use from the ASCII TABLE*/
  void *end_of_str;
};

typedef struct prefix_tree_node *trie_node;

struct trie {
  trie_node root;
};

typedef struct trie *Trie;

/**
 * @brief creates a new trie
 * @return a pointer to a prefix tree data structure
 */
Trie create_trie();
/**
 * @brief Create a trie node
 *
 * @return trie_node
 */
trie_node create_node();

/**
 * @brief
 *
 * @param str the string added to the trie
 * @param trie a pointer to a prefix tree
 * @param end_of_word
 * @return  return a pointer to the end of string context, return NULL if failed
 * to allocate memory
 */
void *insert_to_trie(const char *str, trie_node root, void *end_of_word);

/**
 * @brief free the trie's memory
 *
 * @param trie a pointer to a pointer of a trie
 */
void trie_destroy(Trie t);

/**
 * @brief checking if a string exist in a trie and returning the context
 *
 * @param trie a trie
 * @param str string being searched
 * @return void* returns a pointer to data context,NULL if not exist
 */
void *find_str(trie_node root, const char *str);

#endif