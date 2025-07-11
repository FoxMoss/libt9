#pragma once

#include "../protobuf/t9db.pb-c.h"
#include <stdio.h>

typedef struct {
  T9Database *db;
  size_t index;
  FILE *streamed_file;
} Cache;

Cache init_cache(char *in_memory_file, char *streamed_file);
void free_cache(Cache cache);
StoredToken *cache_find_token(int32_t id, Cache *cache);
StoredTrieNode *cache_get_trie(size_t id, Cache *cache);
StoredTrieNode *cache_simple_search(char *char_ptr, Cache *cache);
