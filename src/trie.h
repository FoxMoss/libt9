#pragma once
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum OptionalType { OPTIONAL_ERROR, OPTIONAL_VALUE };
struct Optional {
  union {
    struct TrieNode *ptr;
    char *err;
  } data;
  enum OptionalType type;
};
struct TrieNode *optional_unwrap(struct Optional *op);

struct Word {
  char *str;
  size_t frequency;
};

enum TrieNodeType { TRIE_CHAR, TRIE_WORD, TRIE_ROOT };
struct TrieNode {
  union {
    char c;
    struct Word w;
  } data;
  enum TrieNodeType type;

  struct TrieNode **children;
  size_t children_len;
};

struct TrieNode *trie_create_word(struct Word word);
struct TrieNode *trie_create_char(char c);
struct TrieNode *trie_create_root();
void trie_appened_child(struct TrieNode *node, struct TrieNode *child);
void trie_free(struct TrieNode *node);
struct Optional trie_get_node(struct TrieNode *root, const char *path);
