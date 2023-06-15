#include "trie.h"
#include <stdio.h>
int main() {
  int i;
  Trie t;
  const char *f;
  const char *strings[4] = {"alon", "alonso", "alonio", "alo"};

  t = create_trie();
  for (i = 0; i < 4; i++) {
    insert_to_trie(strings[i], t->root, (void *)strings[i]);
  }
  for (i = 0; i < 4; i++) {
    f = find_str(t->root, strings[i]);
    if (f != NULL) {
      printf("%s\n", f);
    } else {
      printf("string:'%s' could not be found.\n", strings[i]);
    }
  }

  return 0;
}