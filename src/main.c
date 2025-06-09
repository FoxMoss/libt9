#include "trie.h"
#include "wordlist.h"
#include <stddef.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
  struct TrieNode *root = trie_load_wordlist();

  char *search_str = "843";
  if (argc >= 2)
    search_str = argv[1];

  struct Optional op_node = trie_get_node(root, search_str);
  struct TrieNode *node = optional_unwrap(&op_node);

  for (size_t child_i = 0; child_i < node->children_len; child_i++) {
    struct TrieNode *child = node->children[child_i];
    if (child->type == TRIE_WORD)
      printf("%s : %zu\n", child->data.w.str, child->data.w.frequency);
  }
  trie_free(root);
}
