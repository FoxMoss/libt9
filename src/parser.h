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
Token *cache_find_token(int32_t id, Cache *cache);
TrieNode *cache_get_trie(size_t id, Cache *cache);
