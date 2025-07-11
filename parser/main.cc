#include "sentencepiece_model.pb.h"
#include "t9db.pb.h"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/message_lite.h>
#include <optional>
#include <sentencepiece_processor.h>
#include <stddef.h>
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
  if (c == '\'')
    return true;
  return false;
}

struct MarkovNode {
  std::string node;
  uint32_t freq;
  std::unordered_map<int, uint32_t> children;
};

struct RawStoredWord {
  std::string word;
  uint32_t freq;
};

struct RawStoredTrieNode {
  char path;
  std::vector<RawStoredTrieNode *> children; // auto frees?
  std::vector<int> tokens;
  std::vector<RawStoredWord> words;
  uint32_t freq;
  uint32_t index;
};

std::unordered_map<int, MarkovNode> tokens;
static uint32_t global_index = 0;

void write_trie(std::ofstream *stream, StoredTrieNode *node) {
  std::string serialized = node->SerializeAsString();
  uint32_t serialized_len = serialized.size();
  stream->write((char *)&serialized_len,
                sizeof(uint32_t)); // host should be similar arch to client
  stream->write(serialized.c_str(), serialized.size());
}

void recurse_trie(RawStoredTrieNode *node, T9Database *db,
                  std::ofstream *streamed_file) {
  for (auto child : node->children) {
    recurse_trie(child, db, streamed_file);
  }
  node->index = global_index++;
  auto trie_node = StoredTrieNode();
  trie_node.set_node_index(node->index);
  trie_node.set_character(node->path);
  trie_node.set_freq(node->freq);

  for (auto child : node->children) {
    trie_node.add_children(child->index);
  }
  for (auto token : node->tokens) {
    trie_node.add_tokens(token);
  }
  for (auto word : node->words) {
    auto trie_word = trie_node.add_words();
    printf("%s\n", word.word.c_str());

    trie_word->set_word(word.word);
    trie_word->set_freq(word.freq);
  }

  write_trie(streamed_file, &trie_node);
}

int main(int argc, char *argv[]) {
  if (argc != 6) {
    printf(
        "Usage: %s [model] [infile1] [infile2] [inmemoryfile] [streamedfile]\n",
        "t9parser");
    return 1;
  }
  sentencepiece::SentencePieceProcessor processor;
  const auto status = processor.Load(std::string_view(argv[1]));
  processor.SetEncodeExtraOptions("bos:eos"); // add <s> and </s>.
  if (!status.ok()) {
    printf("%s", status.error_message());
    return 1;
  }

  std::ifstream read_stream(argv[2]);
  std::vector<std::string> lines;
  std::string line;
  while (std::getline(read_stream, line)) {
    lines.push_back(line);
  }
  std::ifstream read_stream2(argv[3]);
  while (std::getline(read_stream2, line)) {
    lines.push_back(line);
  }

  std::unordered_map<std::string, size_t> words;
  for (uint32_t a_i = 0; a_i < lines.size(); a_i++) {
    std::string word;
    for (auto c : lines[a_i]) {
      if (!is_letter(c) && word != "") {
        printf("%s\n", word.c_str());
        words[word]++;
        word = "";
      }

      if (is_letter(c))
        word.push_back(c);
    }
    std::vector<std::string> pieces;
    std::vector<int> ids;
    processor.Encode(lines[a_i], &pieces);
    processor.Encode(lines[a_i], &ids);
    for (uint32_t token_i = 0; token_i < ids.size(); token_i++) {
      if (tokens.find(ids[token_i]) == tokens.end()) {
        tokens[ids[token_i]] = {pieces[token_i], 1, {}};
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
        tokens[last_tok.value()].children[ids[token_i]]++;
      }
      printf("[%s]\n", pieces[token_i].c_str());
      last_tok = ids[token_i];
    }
  }

  std::unordered_map<char, RawStoredTrieNode *> roots;
  sentencepiece::ModelProto model = processor.model_proto();

  for (int id = 0; id < model.pieces_size(); ++id) { // i is id
    const auto &sp = model.pieces(id);
    RawStoredTrieNode *last_node = NULL;
    if (tokens.find(id) == tokens.end()) {
      tokens[id] = {sp.piece(), 0, {}};
    }

    for (char c : sp.piece()) {
      char t9 = char_to_t9(c);
      if (t9 == '\0') {
        continue;
      }
      RawStoredTrieNode *curent_node;
      if (last_node == NULL) {
        if (roots.find(t9) == roots.end()) {
          roots[t9] = new RawStoredTrieNode{t9, {}, {}, {}, 1};
        }
        curent_node = roots[t9];
      } else {
        for (auto child : last_node->children) {
          if (child->path == t9) {
            curent_node = child;
            goto skip_creation;
          }
        }
        curent_node = new RawStoredTrieNode{t9, {}, {}, {}, 1};
        last_node->children.push_back(curent_node);
      }
    skip_creation:

      curent_node->freq++;
      last_node = curent_node;
    }
    if (last_node != NULL) {
      last_node->tokens.push_back(id);
    }
  }

  for (auto word_pair : words) {
    RawStoredTrieNode *last_node = NULL;
    for (char c : word_pair.first) {
      printf("%c\n", c);
      char t9 = char_to_t9(c);
      if (t9 == '\0') {
        continue;
      }
      RawStoredTrieNode *curent_node;
      if (last_node == NULL) {
        if (roots.find(t9) == roots.end()) {
          roots[t9] = new RawStoredTrieNode{t9, {}, {}, {}, 1};
        }
        curent_node = roots[t9];
      } else {
        for (auto child : last_node->children) {
          if (child->path == t9) {
            curent_node = child;
            goto skip_creation_word;
          }
        }
        printf("child created %c\n", t9);
        curent_node = new RawStoredTrieNode{t9, {}, {}, {}, 1};
        last_node->children.push_back(curent_node);
      }
    skip_creation_word:

      curent_node->freq++;
      last_node = curent_node;
    }
    if (last_node != NULL) {
      last_node->words.push_back(
          {word_pair.first, static_cast<uint32_t>(word_pair.second)});
      printf("boped %s\n", word_pair.first.c_str());
    }
  }

  T9Database db;
  for (auto token : tokens) {
    auto tok = db.add_tokens();
    tok->set_id(token.first);
    tok->set_freq(token.second.freq);
    tok->set_value(token.second.node);
    for (auto edge_value : token.second.children) {
      auto edge = tok->add_edges();
      edge->set_target_id(edge_value.first);
      edge->set_freq(edge_value.second);
    }
  }

  std::ofstream trie_stream(argv[5], std::ios::binary);

  for (auto root : roots) {
    for (auto child : root.second->children) {
      recurse_trie(child, &db, &trie_stream);
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
    for (auto word : root.second->words) {
      auto trie_word = trie_node->add_words();
      trie_word->set_word(word.word);
      trie_word->set_freq(word.freq);
    }
  }

  std::ofstream output(argv[4], std::ios::binary);
  google::protobuf::io::OstreamOutputStream output_stream(&output);
  google::protobuf::io::CodedOutputStream coded_output(&output_stream);
  db.SerializeToCodedStream(&coded_output);

  return 0;
}
