#include "trie.h"

#ifndef KERNEL
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#else
#include <linux/hid.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/usb/input.h>
#define printf printk
#define malloc(val) kmalloc(val, GFP_KERNEL)
#define realloc(val1, val2) krealloc(val1, val2, GFP_KERNEL)
#define free(val) kfree(val)
#endif

struct TrieNode *optional_unwrap(struct Optional *op) {
  if (op->type == OPTIONAL_VALUE)
    return op->data.ptr;

  printf("Error: %s\n", op->data.err);

  return NULL;
}

struct TrieNode *trie_create_word(struct Word word) {
  struct TrieNode *ret = (struct TrieNode *)malloc(sizeof(struct TrieNode));
  ret->data.w = word;
  ret->type = TRIE_WORD;
  ret->children = (struct TrieNode **)malloc(0);
  ret->children_len = 0;
  return ret;
}

struct TrieNode *trie_create_char(char c) {
  struct TrieNode *ret = (struct TrieNode *)malloc(sizeof(struct TrieNode));
  ret->data.c = c;
  ret->type = TRIE_CHAR;
  ret->children = (struct TrieNode **)malloc(0);
  ret->children_len = 0;
  return ret;
}

struct TrieNode *trie_create_root(void) {
  struct TrieNode *ret = (struct TrieNode *)malloc(sizeof(struct TrieNode));
  ret->type = TRIE_ROOT;
  ret->children = (struct TrieNode **)malloc(0);
  ret->children_len = 0;
  return ret;
}

void trie_appened_child(struct TrieNode *node, struct TrieNode *child) {
  node->children_len++;
  node->children = (struct TrieNode **)realloc(
      node->children, node->children_len * sizeof(struct TrieNode *));
  node->children[node->children_len - 1] = child;
}

void trie_free(struct TrieNode *node) {
  for (size_t i = 0; i < node->children_len; i++) {
    trie_free(node->children[i]);
  }
  free(node);
}

struct TrieNode *trie_fillout_path(struct TrieNode *root, char *path) {
  size_t path_len = strlen(path);

  struct TrieNode *node = root;
  for (size_t i = 0; i < path_len; i++) {
    for (size_t c = 0; c < node->children_len; c++) {
      if (node->children[c]->type == TRIE_CHAR &&
          node->children[c]->data.c == path[i]) {
        node = node->children[c];
        goto next_char;
      }
    }
    struct TrieNode *parent = node;
    node = trie_create_char(path[i]);
    trie_appened_child(parent, node);
  next_char:
    continue;
  }
  return node;
}

struct Optional trie_get_node(struct TrieNode *root, const char *path) {
  struct Optional ret;
  size_t len = strlen(path);
  struct TrieNode *selected_node = root;
  for (size_t i = 0; i < len; i++) {
    for (size_t child_i = 0; child_i < selected_node->children_len; child_i++) {
      struct TrieNode *child = selected_node->children[child_i];
      if (child->type != TRIE_CHAR)
        continue;

      if (child->data.c == path[i]) {
        selected_node = child;

        goto found;
      }
    }
    goto return_null;
  found:
    continue;
  }

  // selected node should be the end of t9
  ret.data.ptr = selected_node;
  ret.type = OPTIONAL_VALUE;
  return ret;

return_null:
  ret.data.err = "Node not found!";
  ret.type = OPTIONAL_ERROR;
  return ret;
}
