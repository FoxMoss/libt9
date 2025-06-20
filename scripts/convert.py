import re
from tqdm import tqdm

output_file = open('wordlist.h', 'w')

path_preamble = ""
main_section = ""

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
    global main_section
    global path_preamble
    path = "p"
    for num in t9:
        path += num
        if path in trie_computed:
            continue
        path_preamble += f"static struct TrieNode *{path};"
        main_section += f"{path} = trie_create_char('{num}');"
        main_section += f"trie_appened_child({path[:-1]}, {path});"
        trie_computed.add(path)


def fillout_word(path, word, frequency):
    global rolling_id
    global main_section
    rolling_id += 1
    main_section += f"struct Word w{rolling_id} = {{\"{word}\", {frequency}}};\n"
    main_section += f"struct TrieNode * n{rolling_id} = trie_create_word(w{rolling_id});\n"
    main_section += f'trie_appened_child(trie_fillout_path(p, "{path}"), n{rolling_id}); \n'


loadsection = 0


def process_word_list(file_path):
    global loadsection
    global main_section
    global path_preamble
    with open(file_path, 'r') as file:
        lines = file.readlines()
        for line in tqdm(lines, desc="Processing words", unit="word"):
            columns = line.strip().split('\t')
            if len(words_computed) % 2000 == 0:
                loadsection += 1
                if loadsection != 1:
                    main_section += f"trie_load_wordlist{loadsection}();\n"
                    main_section += "return;"
                    main_section += "}"
                path_preamble += f"void trie_load_wordlist{loadsection}();\n"
                main_section += f"void trie_load_wordlist{loadsection}() {'{'}\n"
            if loadsection > 10:
                break
            if len(columns) == 3:
                words, frequency = columns[1], columns[2]
                for word in words.split():
                    cleaned_word = re.sub(r'[^a-zA-Z]', '', word)

                    if cleaned_word in words_computed:
                        continue
                    words_computed.add(cleaned_word)
                    if cleaned_word:
                        t9 = to_t9(cleaned_word)
                        fillout_word(''.join(t9), cleaned_word, frequency)
        main_section += "return;\n"
        main_section += "}\n"


file_path = 'word_list.txt'
process_word_list(file_path)

write_file = """
#include "wordlist.h"

struct TrieNode *p;
"""
write_file += path_preamble
write_file += main_section
write_file += """
struct TrieNode *trie_load_wordlist() {
p = trie_create_root();
trie_load_wordlist1();
return p;
}
"""
output_file.write(write_file)
