#include "trie.h"
#include "wordlist.h"
#include <stddef.h>
#include <stdio.h>

void dump_tree(struct TrieNode *node, size_t layers) {
  for (size_t child_i = 0; child_i < node->children_len; child_i++) {
    struct TrieNode *child = node->children[child_i];
    if (child->type == TRIE_WORD) {
      for (size_t i = 0; i < layers; i++) {
        printf(">");
      }
      printf("%s : %zu\n", child->data.w.str, child->data.w.frequency);
    }
    if (child->type == TRIE_CHAR) {
      for (size_t i = 0; i < layers; i++) {
        printf(">");
      }
      printf("%c\n", child->data.c);
    }
    dump_tree(child, layers + 1);
  }
}

int main(int argc, char *argv[]) {
  struct TrieNode *root = trie_load_wordlist();

  char *search_str = "843";
  if (argc >= 2)
    search_str = argv[1];

  struct Optional op_node = trie_get_node(root, search_str);
  struct TrieNode *node = optional_unwrap(&op_node);
  // struct TrieNode *node = root;
  //
  dump_tree(node, 0);
  // for (size_t child_i = 0; child_i < node->children_len; child_i++) {
  //   struct TrieNode *child = node->children[child_i];
  //   if (child->type == TRIE_WORD)
  //     printf("%s : %zu\n", child->data.w.str, child->data.w.frequency);
  //   if (child->type == TRIE_CHAR)
  //     printf("%c\n", child->data.c);
  // }
  trie_free(root);
}
