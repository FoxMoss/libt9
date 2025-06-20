#include "trie.h"
#include "wordlist.h"
#include <stddef.h>
#include <stdio.h>

static struct TrieNode *root;

int main(int argc, char *argv[]) { root = trie_load_wordlist(); }

char *get_word(char *path, size_t index) {
  struct Optional optional = trie_get_node(root, path);
  if (optional.type == OPTIONAL_ERROR)
    return path;

  struct TrieNode *node = optional.data.ptr;
  if (node->children_len == 0)
    return path;

  int no_words = 0;
  for (size_t i = 0; i < node->children_len; i++) {
    if (node->children[i]->type == TRIE_WORD) {
      no_words++;
    }
  }
  if (no_words == 0) {
    return path;
  }

  size_t skipped = 0;
  size_t current_index = 0;
  do {
    if (node->children[current_index]->type == TRIE_WORD) {
      if (skipped < index) {
        skipped++;
        current_index++;
        current_index %= node->children_len;
        continue;
      }
      return node->children[current_index]->data.w.str;
    }
    current_index++;
    current_index %= node->children_len;
  } while (1);
  return path;
}
