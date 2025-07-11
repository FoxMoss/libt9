#include "parser.h"
#include "../protobuf/t9db.pb-c.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

Cache init_cache(char *in_memory_file, char *streamed_file) {
  FILE *in_memory_fd = fopen(in_memory_file, "r");
  long begin = ftell(in_memory_fd);
  fseek(in_memory_fd, 0, SEEK_END);
  long in_memory_len = ftell(in_memory_fd) - begin;
  fseek(in_memory_fd, 0, SEEK_SET);
  uint8_t *in_memory = malloc(in_memory_len);
  fread(in_memory, in_memory_len, 1, in_memory_fd);

  T9Database *db = t9_database__unpack(NULL, in_memory_len, in_memory);
  if (db == 0) {
    printf("Cannot unpack db.\n");
    exit(1);
  }
  free(in_memory);
  fclose(in_memory_fd);

  FILE *streamed_fd = fopen(streamed_file, "r");

  return (Cache){db, 0, streamed_fd};
}

StoredTrieNode *cache_get_trie(size_t id, Cache *cache) {
  fseek(cache->streamed_file, 0, SEEK_SET);
  cache->index = 0;

  while (cache->index != id) {
    uint32_t size;
    fread(&size, sizeof(uint32_t), 1, cache->streamed_file);
    fseek(cache->streamed_file, size, SEEK_CUR);
    cache->index++;
  }

  cache->index = id;

  uint32_t size;
  fread(&size, sizeof(uint32_t), 1, cache->streamed_file);

  uint8_t buffer[size];
  fread(buffer, size, 1, cache->streamed_file);
  return stored_trie_node__unpack(NULL, size, buffer);
}

StoredToken *cache_find_token(int32_t id, Cache *cache) {
  for (size_t token_i = 0; token_i < cache->db->n_tokens; token_i++) {
    if (cache->db->tokens[token_i]->id == id) {
      return cache->db->tokens[token_i];
    }
  }
  return NULL;
}

StoredTrieNode *cache_simple_search(char *char_ptr, Cache *cache) {
  StoredTrieNode *last_node = NULL;
  StoredTrieNode *current_node = NULL;
  bool last_node_allocd = false;
  bool current_node_allocd = false;
  bool ran_cycle = false;
  while (*char_ptr != 0) {
    ran_cycle = true;
    if (last_node == NULL) {
      for (size_t i = 0; i < cache->db->n_roots; i++) {
        if (cache->db->roots[i]->character == *char_ptr) {
          current_node = cache->db->roots[i];
          break;
        }
      }
    } else {
      for (size_t i = 0; i < last_node->n_children; i++) {
        StoredTrieNode *canidate_node =
            cache_get_trie(last_node->children[i], cache);
        if (canidate_node->character == *char_ptr) {
          current_node = canidate_node;
          current_node_allocd = true;
          break;
        } else {
          stored_trie_node__free_unpacked(canidate_node, NULL);
        }
      }
    }
    if (current_node == NULL) {
      break;
    }

    char_ptr++;
    if (last_node_allocd) {
      stored_trie_node__free_unpacked(last_node, NULL);
    }
    last_node = current_node;
    last_node_allocd = current_node_allocd;
    current_node_allocd = false;
    current_node = NULL;
  }
  return last_node;
}

void free_cache(Cache cache) {
  t9_database__free_unpacked(cache.db, NULL);
  fclose(cache.streamed_file);
}
