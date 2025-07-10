#include "parser.h"
// #include "trie.h"
// #include "wordlist.h"
#include <ncurses.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// bool open_layers[128] = {};
//
// void draw_tail(WINDOW *win, size_t layers_up_to, bool end, size_t *line) {
//   wmove(win, *line, 1);
//
//   if (layers_up_to == 0)
//     return;
//
//   for (size_t i = 1; i < layers_up_to; i++) {
//     if (open_layers[i]) {
//       waddch(win, ACS_VLINE);
//       wprintw(win, " ");
//     } else {
//       wprintw(win, "  ");
//     }
//   }
//
//   if (end) {
//     waddch(win, ACS_LLCORNER);
//     waddch(win, ACS_HLINE);
//   } else {
//     waddch(win, ACS_LTEE);
//     waddch(win, ACS_HLINE);
//   }
// }
//
// void dump_tree(WINDOW *win, struct TrieNode *node, size_t layers,
//                size_t *max_lines, size_t *line, size_t *start_line,
//                int higlight_line) {
//
//   if (layers > 128) {
//     return;
//   }
//   open_layers[layers] = true;
//   for (size_t child_i = 0; child_i < node->children_len; child_i++) {
//     if (child_i == node->children_len - 1)
//       open_layers[layers] = false;
//
//     struct TrieNode *child = node->children[child_i];
//     if (max_lines == 0)
//       return;
//
//     if (*start_line == 0) {
//       *max_lines = *max_lines - 1;
//       *line += 1;
//
//       if (*line == higlight_line) {
//         wattron(win, COLOR_PAIR(3));
//       }
//       draw_tail(win, layers, child_i == node->children_len - 1, line);
//
//       if (child->type == TRIE_WORD) {
//         wprintw(win, "\"%s\"\n", child->data.w.str);
//       }
//       if (child->type == TRIE_CHAR) {
//         wprintw(win, "%c\n", child->data.c);
//       }
//       if (*line == higlight_line) {
//         wattroff(win, COLOR_PAIR(3));
//       }
//
//     } else {
//       *start_line = *start_line - 1;
//     }
//
//     dump_tree(win, child, layers + 1, max_lines, line, start_line,
//               higlight_line);
//   }
// }
//
// static struct TrieNode *root;
// char *get_word(char *path, size_t index) {
//   struct Optional optional = trie_get_node(root, path);
//   if (optional.type == OPTIONAL_ERROR)
//     return path;
//
//   struct TrieNode *node = optional.data.ptr;
//   if (node->children_len == 0)
//     return path;
//
//   int no_words = 0;
//   for (size_t i = 0; i < node->children_len; i++) {
//     if (node->children[i]->type == TRIE_WORD) {
//       no_words++;
//     }
//   }
//   if (no_words == 0) {
//     return path;
//   }
//
//   size_t skipped = 0;
//   size_t current_index = 0;
//   do {
//     if (node->children[current_index]->type == TRIE_WORD) {
//       if (skipped < index) {
//         skipped++;
//         current_index++;
//         current_index %= node->children_len;
//         continue;
//       }
//       return node->children[current_index]->data.w.str;
//     }
//     current_index++;
//     current_index %= node->children_len;
//   } while (1);
//   return path;
// }
//
// struct MenuOption {
//   char *str;
//   enum { MENU_TOGGLE, MENU_CALLBACK } type;
//   union {
//     bool toggle_value;
//     void (*callback)();
//   } data;
// };
//
// void quit() {
//   trie_free(root);
//   endwin();
//   exit_curses(1);
//   exit(1);
// }
// #define menu_len 3
// struct MenuOption menu[menu_len] = {
//     {.str = "Flip numpad to match phone keypad",
//      .type = MENU_TOGGLE,
//      .data = {.toggle_value = false}},
//     {.str = "Provide a cheatsheet while typing",
//      .type = MENU_TOGGLE,
//      .data = {.toggle_value = false}},
//     {.str = "Quit", .type = MENU_CALLBACK, .data = {.callback = quit}}};
//
// // must update every time menu items update (increment?)
// #define T9_MAGIC_VER 0x54, 0x09, 0x01
// void save_config(char *filename) {
//   FILE *file = fopen(filename, "w");
//   if (file == NULL)
//     return;
//   char magic[] = {T9_MAGIC_VER};
//
//   fwrite((void *)&magic, 3, 1, file);
//   for (size_t i = 0; i < menu_len; i++) {
//     if (menu[i].type == MENU_TOGGLE) {
//       fwrite(&i, sizeof(size_t), 1, file);
//       // 8 bytes, or maybe not :P
//       fwrite(&menu[i].data.toggle_value, 1, 1, file); // one byte
//     }
//   }
//   fclose(file);
// }
// void read_config(char *filename) {
//   FILE *file = fopen(filename, "r");
//   if (file == NULL)
//     return;
//   char magic[3] = {T9_MAGIC_VER};
//
//   char magic_check[3];
//   fread(magic_check, 3, 1, file);
//   if (memcmp(magic, magic_check, 3) != 0) // doesnt match magic
//     return;
//
//   struct Item {
//     size_t index;
//     char toggle;
//   } __attribute__((packed)) item;
//
//   while (fread(&item, sizeof(struct Item), 1, file) != 0) {
//     if (item.index >= menu_len || menu[item.index].type != MENU_TOGGLE)
//       return;
//     menu[item.index].data.toggle_value = item.toggle != 0;
//   }
//   fclose(file);
// }
//
// static size_t menu_selector = 0;
//
// void draw_cheatsheet(WINDOW *cheatsheet) {
//   box(cheatsheet, 0, 0);
//   wmove(cheatsheet, 0, 2);
//   wprintw(cheatsheet, "Cheatsheet");
//   for (size_t x = 0; x < 3; x++) {
//     for (size_t y = 0; y < 4; y++) {
//       size_t num = 1 + x + y * 3;
//
//       wmove(cheatsheet, y * 5 + 2, x * 6 + 3);
//       if (num <= 9) {
//         wprintw(cheatsheet, "%zu", num);
//       } else if (num == 10) {
//         wprintw(cheatsheet, "*");
//       } else if (num == 11) {
//         wprintw(cheatsheet, "0");
//       } else if (num == 12) {
//         wprintw(cheatsheet, "-");
//       }
//
//       wmove(cheatsheet, y * 5 + 4, x * 6 + 3);
//       if (1 < num && num < 7) {
//         wprintw(cheatsheet, "%c%c%c", (char)num * 3 + 'A' - 6,
//                 (char)num * 3 + 'A' - 5, (char)num * 3 + 'A' - 4);
//       } else if (num == 7) {
//         wprintw(cheatsheet, "PQRS");
//       } else if (num == 8) {
//         wprintw(cheatsheet, "TUV");
//       } else if (num == 9) {
//         wprintw(cheatsheet, "WXYZ");
//       }
//     }
//   }
// }
//
size_t high_score = 0;
int32_t *high_array;

void recurse_trie(int32_t *current_buffer, size_t current_buffer_len,
                  int32_t appened_token, char *char_ptr, size_t score_count,
                  Cache *cache) {
  TrieNode *last_node = NULL;
  TrieNode *current_node = NULL;
  // TODO: free current_node
  size_t score_count_copy = score_count;
  int32_t *current_buffer_copy = current_buffer;
  if (appened_token != -1) {
    Token *canidate_token = cache_find_token(appened_token, cache);
    if (current_buffer_len > 1 &&
        strncmp(canidate_token->value, "▁", strlen("▁")) == 0  ) 
      return;

    current_buffer_copy = malloc(current_buffer_len);
    for (size_t i = 0; i < current_buffer_len - 1; i++) {
      current_buffer_copy[i] = current_buffer[i];
    }
    current_buffer_copy[current_buffer_len - 1] = appened_token;

    if (canidate_token == NULL) {
      printf("fatal error could not find token %i\n", appened_token);
      exit(1);
    }
    if (current_buffer_len >= 2) {
      Token *parent_token =
          cache_find_token(current_buffer_copy[current_buffer_len - 2], cache);
      for (size_t i = 0; i < parent_token->n_edges; i++) {
        if (parent_token->edges[i]->target_id == appened_token) {
          score_count_copy += parent_token->edges[i]->freq;
          break;
        }
      }
    }

    score_count_copy += canidate_token->freq;
  }
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
        TrieNode *canidate_node = cache_get_trie(last_node->children[i], cache);
        if (canidate_node->character == *char_ptr) {
          current_node = canidate_node;
          break;
        }
      }
    }
    if (current_node == NULL) {
      printf("res failed to get tokens\n");
      return;
    }

    for (size_t i = 0; i < current_node->n_tokens; i++) {
      if (appened_token != -1) {
        Token *token_chunk = cache_find_token(appened_token, cache);
        bool in_prev = false;
        for (size_t prev_i = 0; prev_i < token_chunk->n_edges; prev_i++) {
          if (token_chunk->edges[prev_i]->target_id ==
              current_node->tokens[i]) {
            in_prev = true;
            break;
          }
        }
        if (!in_prev) {
          break;
        }
      }
      recurse_trie(current_buffer_copy, current_buffer_len + 1,
                   current_node->tokens[i], char_ptr + 1, score_count_copy,
                   cache);
    }

    score_count_copy += current_node->freq;
    char_ptr++;
    last_node = current_node;
    current_node = NULL;
  }

  if (high_score < score_count &&
      !ran_cycle) { // code only runs a cycle if not in an end token state
    for (size_t i = 0; i < current_buffer_len; i++) {
      Token *token_chunk = cache_find_token(current_buffer_copy[i], cache);

      printf("[%s]", token_chunk->value);
    }
    printf(": %zu\n", score_count / current_buffer_len);
  }
  free(current_buffer_copy);
}

int main(int argc, char *argv[]) {
  Cache cache = init_cache("inmemory.t9db", "streamed.t9stream");
  char *target_num = "528474";
  size_t target_num_len = strlen(target_num);

  recurse_trie(NULL, 0, -1, target_num, 0, &cache);

  free_cache(cache);

  // root = trie_load_wordlist();
  // read_config("/tmp/.t9");
  //
  // initscr();
  // mousemask(ALL_MOUSE_EVENTS, NULL);
  // cbreak();
  //
  // start_color();
  // init_pair(1, COLOR_CYAN, COLOR_BLACK);
  // init_pair(2, COLOR_WHITE, COLOR_BLACK);
  // init_pair(3, COLOR_BLACK, COLOR_WHITE);
  //
  // int max_y = getmaxy(stdscr);
  // int max_x = getmaxx(stdscr);
  // keypad(stdscr, true);
  // noecho();
  //
  // WINDOW *win = newwin(3, 0, 0, 0);
  // WINDOW *tree = newwin(0, 0, 3, 0);
  // WINDOW *options = newwin(4 + menu_len, max_x - 10, 5, 5);
  // WINDOW *cheatsheet = newwin(24, 21, 1, max_x - 23);
  //
  // char current_word_buffer[256] = {};
  // char *str = malloc(0);
  // size_t cursor = 0;
  // size_t start_line = 0;
  // size_t index = 0;
  // int x, y = 0;
  // bool settings = false;
  //
  // while (true) {
  //
  //   wclear(tree);
  //
  //   struct TrieNode *path = trie_fillout_path(root, current_word_buffer);
  //   size_t max_lines = max_y - 5;
  //   size_t line = 0;
  //   size_t start_line_copy = start_line;
  //   wmove(tree, 1, 2);
  //
  //   if (!settings) {
  //     dump_tree(tree, path, 0, &max_lines, &line, &start_line_copy, y - 3);
  //   } else {
  //     wprintw(tree, "tree hidden...");
  //   }
  //
  //   wattron(win, COLOR_PAIR(settings + 1));
  //   wattron(tree, COLOR_PAIR(settings + 1));
  //   box(win, 0, 0);
  //   wmove(win, 0, 2);
  //   wprintw(win, "Input");
  //
  //   box(tree, 0, 0);
  //   wmove(tree, 0, 2);
  //   wprintw(tree, "Tree");
  //   wattroff(win, COLOR_PAIR(settings + 1));
  //   wattroff(tree, COLOR_PAIR(settings + 1));
  //
  //   if (menu[1].data.toggle_value) {
  //     draw_cheatsheet(cheatsheet);
  //   }
  //
  //   clear();
  //   refresh();
  //   wrefresh(win);
  //   wrefresh(tree);
  //   wrefresh(cheatsheet);
  //
  //   wattron(win, COLOR_PAIR(settings + 1));
  //   wattron(tree, COLOR_PAIR(settings + 1));
  //   move(1, 1);
  //   printw("%s", str);
  //   printw("%s", get_word(current_word_buffer, index));
  //   wattroff(win, COLOR_PAIR(settings + 1));
  //   wattroff(tree, COLOR_PAIR(settings + 1));
  //
  //   if (settings) {
  //     wattron(options, COLOR_PAIR(1));
  //     wclear(options);
  //     box(options, 0, 0);
  //     wmove(options, 0, 2);
  //     wprintw(options, "Options");
  //
  //     for (size_t i = 0; i < menu_len; i++) {
  //       wmove(options, 2 + i, 1);
  //       struct MenuOption *opt = &menu[i];
  //       if (opt->type == MENU_TOGGLE) {
  //         if (opt->data.toggle_value)
  //           wprintw(options, "ON");
  //         else
  //           wprintw(options, "OFF");
  //         wprintw(options, " - ");
  //         wprintw(options, "%s", opt->str);
  //       } else if (opt->type == MENU_CALLBACK) {
  //         wprintw(options, "%s", opt->str);
  //       }
  //     }
  //     move(7 + menu_selector, 6);
  //
  //     wrefresh(options);
  //     wattroff(options, COLOR_PAIR(1));
  //   }
  //
  //   int c = getch();
  //   if (settings) {
  //     switch (c) {
  //     case 27: // escape key
  //     case '+': {
  //       settings = !settings;
  //       break;
  //     }
  //
  //     case '\n': {
  //       struct MenuOption *opt = &menu[menu_selector];
  //       if (opt->type == MENU_TOGGLE) {
  //         opt->data.toggle_value = !opt->data.toggle_value;
  //       } else if (opt->type == MENU_CALLBACK) {
  //         opt->data.callback();
  //       }
  //
  //       break;
  //     }
  //     case 'j':
  //     case KEY_DOWN: {
  //       menu_selector += 1;
  //       menu_selector %= menu_len;
  //       break;
  //     }
  //     case 'k':
  //     case KEY_UP: {
  //       if (menu_selector == 0) {
  //         menu_selector = menu_len - 1;
  //         break;
  //       }
  //       menu_selector -= 1;
  //
  //       break;
  //     }
  //     }
  //     save_config("/tmp/.t9");
  //   } else {
  //
  //     switch (c) {
  //     case 27: // escape key
  //     case '+': {
  //       settings = !settings;
  //       break;
  //     }
  //
  //     case '*': {
  //       index++;
  //       break;
  //     }
  //     case '0': {
  //       char *current_word = get_word(current_word_buffer, index);
  //       size_t current_word_len = strlen(current_word) + 1;
  //       size_t previous_len = strlen(str) + 1;
  //       str = realloc(str, previous_len + current_word_len);
  //       strncat(str, current_word, current_word_len);
  //       strncat(str, " ", 1);
  //
  //       cursor = 0;
  //       current_word_buffer[0] = 0;
  //       index = 0;
  //       break;
  //     }
  //     case '1':
  //     case '2':
  //     case '3':
  //     case '4':
  //     case '5':
  //     case '6':
  //     case '7':
  //     case '8':
  //     case '9': {
  //       if (menu[0].data.toggle_value) {
  //         if (c >= '7' && c <= '9')
  //           c = c - '7' + '1';
  //         else if (c >= '1' && c <= '3')
  //           c = c - '1' + '7';
  //       }
  //       current_word_buffer[cursor++] = c;
  //       current_word_buffer[cursor] = 0;
  //       if (cursor == 256) {
  //         cursor--;
  //       }
  //       start_line = 0;
  //       index = 0;
  //       break;
  //     }
  //     case '-':
  //     case KEY_BACKSPACE: {
  //       if (cursor != 0) {
  //         cursor--;
  //       }
  //       current_word_buffer[cursor] = 0;
  //       break;
  //     }
  //     case 'k':
  //     case KEY_UP: {
  //       if (start_line == 0)
  //         break;
  //       start_line--;
  //       break;
  //     }
  //     case 'j':
  //     case KEY_DOWN: {
  //       start_line++;
  //       break;
  //     }
  //     case KEY_MOUSE: {
  //       MEVENT event;
  //       if (getmouse(&event) == OK) {
  //         if (event.bstate & BUTTON1_PRESSED) {
  //           x = event.x;
  //           y = event.y;
  //         }
  //         if (event.bstate & BUTTON1_RELEASED) {
  //           int res = start_line + y - event.y;
  //           y = 0;
  //           if (res < 0) {
  //             start_line = 0;
  //             break;
  //           }
  //           start_line = res;
  //         }
  //       }
  //       break;
  //     }
  //
  //     default: {
  //       break;
  //     }
  //     }
  //   }
  // }
  //
  // endwin();
}
