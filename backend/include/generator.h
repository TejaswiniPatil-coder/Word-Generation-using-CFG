#ifndef GENERATOR_H
#define GENERATOR_H

#include "grammar.h"

#define MAX_WORDS 200
#define MAX_WORD_LEN 100
#define MAX_DEPTH 20

typedef struct {
    char words[MAX_WORDS][MAX_WORD_LEN];
    int count;
} WordList;

void generate_words(const Grammar *g, int max_length, WordList *wl);
void generate_from_symbol(const Grammar *g, const char *current, int depth, int max_len, WordList *wl);

#endif
