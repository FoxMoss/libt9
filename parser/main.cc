#include "../protobuf/t9db.pb.h"
#include "sentencepiece_model.pb.h"
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <google/protobuf/any.h>
#include <optional>
#include <sentencepiece_processor.h>
#include <string>
#include <unistd.h>
#include <unordered_map>
#include <vector>

char char_to_t9(char c) {
  switch (tolower(c)) {
  case 'a':
  case 'b':
  case 'c':
    return '2';
  case 'd':
  case 'e':
  case 'f':
    return '3';
  case 'g':
  case 'h':
  case 'i':
    return '4';
  case 'j':
  case 'k':
  case 'l':
    return '5';
  case 'm':
  case 'n':
  case 'o':
    return '6';
  case 'p':
  case 'q':
  case 'r':
  case 's':
    return '7';
  case 't':
  case 'u':
  case 'v':
    return '8';
  case 'w':
  case 'x':
  case 'y':
  case 'z':
    return '9';
  default:
    return '\0';
  }
}

// https://en.wikipedia.org/wiki/Jenkins_hash_function
uint32_t hash_func(const char *key, uint32_t length) {
  uint32_t i = 0;
  uint32_t hash = 0;
  while (i != length) {
    hash += key[i++];
    hash += hash << 10;
    hash ^= hash >> 6;
  }
  hash += hash << 3;
  hash ^= hash >> 11;
  hash += hash << 15;
  return hash;
}

std::vector<std::string> str_overlap(std::string a, std::string b) {
  std::vector<std::string> ret;
  for (uint32_t a_i = 0; a_i < a.size(); a_i++) {
    for (uint32_t b_i = 0; b_i < a.size(); b_i++) {
      std::string match = "";
      while (a_i < a.size() && b_i < b.size() && a[a_i] == b[b_i]) {
        match.push_back(a[a_i]);
        a_i++;
        b_i++;
      }
      if (match != "" && match.size() > 1)
        ret.push_back(match);
    }
  }
  return ret;
}
bool str_starts_with(std::string str, std::string pattern) {
  if (pattern.size() > str.size())
    return false;
  for (uint32_t i = 0; i < pattern.size(); i++)
    if (pattern[i] != str[i])
      return false;
  return true;
}

bool is_letter(char c) {
  if (c >= 'a' && c <= 'z')
    return true;
  if (c >= 'A' && c <= 'Z')
    return true;
  if (c == ' ')
    return true;
  return false;
}

struct MarkovNode {
  std::string node;
  uint32_t freq;
  std::unordered_map<int, uint32_t> children;
};

struct RawTrieNode {
  char path;
  std::vector<RawTrieNode *> children; // auto frees?
  std::vector<int> tokens;
  uint32_t freq;
  uint32_t index;
};

std::unordered_map<int, MarkovNode> tokens;
static uint32_t global_index = 0;
void recurse_trie(RawTrieNode *node, T9Database *db) {
  for (auto child : node->children) {
    recurse_trie(child, db);
  }
  node->index = global_index++;
  auto trie_node = db->add_trie_memory();
  trie_node->set_node_index(node->index);
  trie_node->set_character(node->path);
  trie_node->set_freq(node->freq);

  for (auto child : node->children) {
    trie_node->add_children(child->index);
  }
  for (auto token : node->tokens) {
    trie_node->add_tokens(token);
  }
};

int main(int argc, char *argv[]) {
  if (argc != 4) {
    printf("Usage: %s [model] [infile] [outfile]\n", "t9parser");
    return 1;
  }
  std::ifstream read_stream(argv[2]);
  sentencepiece::SentencePieceProcessor processor;
  const auto status = processor.Load(std::string_view(argv[1]));
  if (!status.ok()) {
    printf("%s", status.error_message());
    return 1;
  }

  std::vector<std::string> lines;
  std::string tweet_context;
  while (std::getline(read_stream, tweet_context)) {
    // if (lines.size() > 100)
    //   break;

    uint32_t comma_count = 0;
    std::string tweet;
    for (auto c : tweet_context) {
      // 6 commas before  what we want
      if (comma_count >= 6 && is_letter(c))
        tweet.push_back(c);
      if (c == '.' && tweet != "") {
        lines.push_back(tweet);
        tweet = "";
      }
      if (c == ',')
        comma_count++;
    }
    lines.push_back(tweet);
  }
  for (uint32_t a_i = 0; a_i < lines.size(); a_i++) {
    std::vector<std::string> pieces;
    std::vector<int> ids;
    processor.Encode(lines[a_i], &pieces);
    processor.Encode(lines[a_i], &ids);
    for (uint32_t token_i = 0; token_i < ids.size(); token_i++) {
      if (tokens.find(ids[token_i]) == tokens.end()) {
        tokens[ids[token_i]] = {pieces[token_i], 1, {}};
        printf("%s\n", pieces[token_i].c_str());
      }
      tokens[ids[token_i]].freq++;
    }
  }

  for (auto pair : tokens) {
    printf("%s : %u\n", pair.second.node.c_str(), pair.second.freq);
  }

  for (uint32_t a_i = 0; a_i < lines.size(); a_i++) {
    printf("Chaining words %f\n", (float)a_i / lines.size() * 100);
    std::string a = lines[a_i];

    std::optional<int> last_tok;
    std::vector<std::string> pieces;
    std::vector<int> ids;
    processor.Encode(lines[a_i], &pieces);
    processor.Encode(lines[a_i], &ids);

    for (uint32_t token_i = 0; token_i < ids.size(); token_i++) {
      if (tokens.find(ids[token_i]) == tokens.end()) {
        tokens[ids[token_i]] = {pieces[token_i], 1, {}};
        printf("%s\n", pieces[token_i].c_str());
      }

      if (last_tok.has_value()) {
        tokens[last_tok.value()].children[last_tok.value()]++;
      }
      printf("[%s]\n", pieces[token_i].c_str());
      last_tok = ids[token_i];
    }
  }

  std::unordered_map<char, RawTrieNode *> roots;
  sentencepiece::ModelProto model = processor.model_proto();

  for (int id = 0; id < model.pieces_size(); ++id) { // i is id
    const auto &sp = model.pieces(id);
    RawTrieNode *last_node = NULL;
    if (tokens.find(id) == tokens.end()) {
      tokens[id] = {sp.piece(), 0, {}};
    }

    for (char c : sp.piece()) {
      char t9 = char_to_t9(c);
      RawTrieNode *curent_node;
      if (last_node == NULL) {
        if (roots.find(t9) == roots.end()) {
          roots[t9] = new RawTrieNode{t9, {}, {}, 0};
        }
        curent_node = roots[t9];
      } else {
        for (auto child : last_node->children) {
          if (child->path == t9) {
            curent_node = child;
            goto skip_creation;
          }
        }
        curent_node = new RawTrieNode{t9, {}, {}, 0};
      }
    skip_creation:

      curent_node->freq++;
      last_node->children.push_back(curent_node);
      last_node = curent_node;
    }
    if (last_node != NULL) {
      last_node->tokens.push_back(id);
    }
  }

  T9Database db;
  for (auto token : tokens) {
    auto tok = db.add_tokens();
    tok->set_id(token.first);
    tok->set_freq(token.second.freq);
    for (auto edge_value : token.second.children) {
      auto edge = tok->add_edges();
      edge->set_target_id(edge_value.first);
      edge->set_freq(edge_value.second);
    }
  }

  for (auto root : roots) {
    for (auto child : root.second->children) {
      recurse_trie(child, &db);
    }
    auto trie_node = db.add_roots();
    trie_node->set_character(root.second->path);
    trie_node->set_freq(root.second->freq);

    for (auto child : root.second->children) {
      trie_node->add_children(child->index);
    }
    for (auto token : root.second->tokens) {
      trie_node->add_tokens(token);
    }
  }

  std::ofstream outfile(argv[3]);
  outfile << db.SerializeAsString();
  outfile.close();

  // RawTrieNode *last_node = NULL;
  //   if (token.first == "[start_token]" || token.first == "[end_token]")
  //     continue;
  //
  //   for (char c : token.first) {
  //     char t9 = char_to_t9(c);
  //     RawTrieNode *curent_node;
  //     if (last_node == NULL) {
  //       if (roots.find(t9) == roots.end()) {
  //         roots[t9] = new RawTrieNode{t9, {}, {}};
  //       }
  //       curent_node = roots[t9];
  //     } else {
  //       for (auto child : last_node->children) {
  //         if (child->path == t9) {
  //           curent_node = child;
  //           goto skip_creation;
  //         }
  //       }
  //       curent_node = new RawTrieNode{t9, {}, {}};
  //       last_node->children.push_back(curent_node);
  //     }
  //   skip_creation:
  //
  //     last_node = curent_node;
  //   }
  //   if (last_node != NULL) {
  //     last_node->tokens.push_back(token.first);
  //   }
  // }
  //
  // // generate lookup tables
  // for (auto &token : tokens) {
  //   uint32_t length = token.first.size();
  //   uint32_t zero = 0;
  //   uint32_t hash = hash_func(token.first.c_str(), token.first.size());
  //   token.second.hash = hash;
  //   fwrite((char *)&hash, sizeof(uint32_t), 1, outfile);
  //   fwrite((char *)&length, sizeof(uint32_t), 1, outfile);
  //   fwrite(token.first.c_str(), token.first.size(), 1, outfile);
  //   fwrite((char *)&token.second.freq, sizeof(uint32_t), 1, outfile);
  //   token.second.offset = ftell(outfile);
  //   fwrite((char *)&zero, sizeof(uint32_t), 1,
  //          outfile); // gets overwritten by ptr
  // }
  // fwrite((char *)&end_array, sizeof(uint32_t), 1, outfile);
  // for (auto token : tokens) {
  //   uint32_t current_loc = ftell(outfile);
  //   fseek(outfile, token.second.offset, SEEK_SET);
  //   fwrite((char *)&current_loc, sizeof(uint32_t), 1,
  //          outfile); // gets overwritten by ptr
  //   fseek(outfile, current_loc, SEEK_SET);
  //
  //   for (auto child : token.second.children) {
  //     uint32_t hash = hash_func(child.first.c_str(), child.first.size());
  //     fwrite((char *)&hash, sizeof(uint32_t), 1, outfile);
  //     fwrite((char *)&child.second, sizeof(uint32_t), 1, outfile);
  //   }
  //   fwrite((char *)&end_array, sizeof(uint32_t), 1, outfile);
  // }
  //
  // for (auto root : roots) {
  //   recurse_trie(root.second, outfile);
  // }
  //
  // fclose(outfile);
  //
  // printf("end_array = %u\n", end_array);

  return 0;
}
