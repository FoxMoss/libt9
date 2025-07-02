#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <functional>
#include <optional>
#include <sentencepiece_processor.h>
#include <string>
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
  uint32_t hash;
  std::unordered_map<std::string, uint32_t> children;
  uint32_t offset;
};

struct TrieNode {
  char path;
  std::vector<TrieNode *> children; // auto frees?
  std::vector<std::string> tokens;
  uint32_t offset;
};

std::unordered_map<std::string, MarkovNode> tokens;
uint32_t end_array = hash_func("[end_array]", strlen("[end_array]"));
void recurse_trie(TrieNode *node, FILE *file) {
  for (auto child : node->children) {
    recurse_trie(child, file);
  }
  node->offset = ftell(file);
  fwrite((char *)&node->path, sizeof(char), 1, file);
  for (auto child : node->children) {
    fwrite((char *)&child->offset, sizeof(uint32_t), 1, file);
  }
  fwrite((char *)&end_array, sizeof(uint32_t), 1, file);
  for (auto token : node->tokens) {
    fwrite((char *)&tokens[token].hash, sizeof(uint32_t), 1, file);
  }
  fwrite((char *)&end_array, sizeof(uint32_t), 1, file);
};

int main(int argc, char *argv[]) {
  if (argc != 4) {
    printf("Usage: %s [model] [infile] [outfile]\n", "t9parser");
    return 1;
  }
  std::ifstream read_stream(argv[1]);
  sentencepiece::SentencePieceProcessor processor;
  const auto status = processor.Load(argv[3]);
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
      if (comma_count >= 6)
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
    processor.Encode(lines[a_i], &pieces);
    for (const std::string &token : pieces) {
      printf("%s\n", token.c_str());
      tokens[token]++;
    }
  }

  for (auto pair : tokens) {
    printf("%s : %u\n", pair.first.c_str(), pair.second.freq);
  }

  for (uint32_t a_i = 0; a_i < lines.size(); a_i++) {
    std::string a = lines[a_i];
    printf("Chaining words %f\n", (float)a_i / lines.size() * 100);

    uint32_t line_i = 0;
    std::string buffer = "";
    std::string last_tok = "[start_token]";
    while (line_i != a.size()) {
      buffer = "";
      if (a[line_i] == ' ') {
        buffer = " ";
        line_i++;
        goto found_token;
      }
      {
        std::optional<std::string> longest_match;
        for (auto token : tokens) {
          if (str_starts_with(a.substr(line_i, a.size() - line_i),
                              token.first)) {
            if (!longest_match.has_value()) {
              longest_match = token.first;
              continue;
            }
            if (longest_match->size() < a.size()) {
              longest_match = token.first;
            }
          }
        }
        if (longest_match.has_value()) {
          line_i += longest_match.value().size();
          buffer = longest_match.value();
          goto found_token;
        }
      }
      while (line_i != a.size() && a[line_i] != ' ') {
        buffer.push_back(a[line_i]);
        line_i++;
      }
    found_token:
      if (tokens.find(last_tok) == tokens.end()) {
        tokens[last_tok].node = last_tok;
        tokens[last_tok].freq = 1;
      }
      tokens[last_tok].children[buffer]++;
      printf("[%s]\n", buffer.c_str());
      last_tok = buffer;
    }
    if (tokens.find(last_tok) == tokens.end()) {
      tokens[last_tok].node = last_tok;
      tokens[last_tok].freq = 1;
    }
    tokens[last_tok].children["[end_token]"]++;
  }

  std::unordered_map<char, TrieNode *> roots;
  TrieNode *last_node = NULL;
  for (auto token : tokens) {
    if (token.first == "[start_token]" || token.first == "[end_token]")
      continue;

    for (char c : token.first) {
      char t9 = char_to_t9(c);
      TrieNode *curent_node;
      if (last_node == NULL) {
        if (roots.find(t9) == roots.end()) {
          roots[t9] = new TrieNode{t9, {}, {}};
        }
        curent_node = roots[t9];
      } else {
        for (auto child : last_node->children) {
          if (child->path == t9) {
            curent_node = child;
            goto skip_creation;
          }
        }
        curent_node = new TrieNode{t9, {}, {}};
        last_node->children.push_back(curent_node);
      }
    skip_creation:

      last_node = curent_node;
    }
    if (last_node != NULL) {
      last_node->tokens.push_back(token.first);
    }
  }

  FILE *outfile = fopen(argv[2], "w");
  // generate lookup tables
  for (auto &token : tokens) {
    uint32_t length = token.first.size();
    uint32_t zero = 0;
    uint32_t hash = hash_func(token.first.c_str(), token.first.size());
    token.second.hash = hash;
    fwrite((char *)&hash, sizeof(uint32_t), 1, outfile);
    fwrite((char *)&length, sizeof(uint32_t), 1, outfile);
    fwrite(token.first.c_str(), token.first.size(), 1, outfile);
    fwrite((char *)&token.second.freq, sizeof(uint32_t), 1, outfile);
    token.second.offset = ftell(outfile);
    fwrite((char *)&zero, sizeof(uint32_t), 1,
           outfile); // gets overwritten by ptr
  }
  fwrite((char *)&end_array, sizeof(uint32_t), 1, outfile);
  for (auto token : tokens) {
    uint32_t current_loc = ftell(outfile);
    fseek(outfile, token.second.offset, SEEK_SET);
    fwrite((char *)&current_loc, sizeof(uint32_t), 1,
           outfile); // gets overwritten by ptr
    fseek(outfile, current_loc, SEEK_SET);

    for (auto child : token.second.children) {
      uint32_t hash = hash_func(child.first.c_str(), child.first.size());
      fwrite((char *)&hash, sizeof(uint32_t), 1, outfile);
      fwrite((char *)&child.second, sizeof(uint32_t), 1, outfile);
    }
    fwrite((char *)&end_array, sizeof(uint32_t), 1, outfile);
  }

  for (auto root : roots) {
    recurse_trie(root.second, outfile);
  }

  fclose(outfile);

  printf("end_array = %u\n", end_array);

  return 0;
}
