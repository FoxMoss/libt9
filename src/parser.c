#include "parser.h"
#include "../protobuf/t9db.pb-c.h"
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

  for (size_t i = 0; i < db->n_tokens; i++) {
    printf("%s\n", db->tokens[i]->value);
  }

  FILE *streamed_fd = fopen(streamed_file, "r");

  return (Cache){db, 0, streamed_fd};
}

TrieNode *cache_get_trie(size_t id, Cache *cache) {
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
  return trie_node__unpack(NULL, size, buffer);
}

Token *cache_find_token(int32_t id, Cache *cache) {
  for (size_t token_i = 0; token_i < cache->db->n_tokens; token_i++) {
    if (cache->db->tokens[token_i]->id == id) {
      return cache->db->tokens[token_i];
    }
  }
  return NULL;
}

void free_cache(Cache cache) {
  t9_database__free_unpacked(cache.db, NULL);
  fclose(cache.streamed_file);
}
