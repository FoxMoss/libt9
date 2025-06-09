import re
from tqdm import tqdm

write_file = """
#include "wordlist.h"

struct TrieNode *trie_load_wordlist() {
struct TrieNode * p = trie_create_root();
"""
output_file = open('wordlist.h', 'w')

t9_map = {
    2: ["a", "b", "c"],
    3: ["d", "e", "f"],
    4: ["g", "h", "i"],
    5: ["j", "k", "l"],
    6: ["m", "n", "o"],
    7: ["p", "q", "r", "s"],
    8: ["t", "u", "v"],
    9: ["w", "x", "y", "z"]
}

char_to_t9 = {char: str(num) for num, chars in t9_map.items()
              for char in chars}

trie_computed = set()
words_computed = set()
rolling_id = 0


def to_t9(word):
    return [char_to_t9[char] for char in word.lower() if char in char_to_t9]


def fillout_trie(t9):
    global trie_computed
    ret = ""
    path = "p"
    for num in t9:
        path += num
        if path in trie_computed:
            continue
        ret += f"struct TrieNode *{path} = trie_create_char('{num}');"
        ret += f"trie_appened_child({path[:-1]}, {path});"
        trie_computed.add(path)
    return ret


def fillout_word(path, word, frequency):
    global rolling_id
    rolling_id += 1
    return (f"struct Word w{rolling_id} = {{\"{word}\", {frequency}}};"
            f"struct TrieNode * n{rolling_id} = trie_create_word(w{rolling_id});"
            f"trie_appened_child({path}, n{rolling_id});")


def process_word_list(file_path):
    global write_file
    with open(file_path, 'r') as file:
        lines = file.readlines()
        for line in tqdm(lines, desc="Processing words", unit="word"):
            columns = line.strip().split('\t')
            if len(columns) == 3:
                words, frequency = columns[1], columns[2]
                for word in words.split():
                    if word in words_computed:
                        continue
                    words_computed.add(word)

                    cleaned_word = re.sub(r'[^a-zA-Z]', '', word)
                    if cleaned_word:
                        t9 = to_t9(cleaned_word)
                        write_file += fillout_trie(t9)
                        write_file += fillout_word('p' +
                                                   ''.join(t9), cleaned_word, frequency)


file_path = 'word_list.txt'
process_word_list(file_path)

write_file += "return p;}"
output_file.write(write_file)
