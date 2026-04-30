#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/grammar.h"
#include "../include/generator.h"

static int word_exists(WordList *wl, const char *w) {
    for (int i = 0; i < wl->count; i++)
        if (strcmp(wl->words[i], w) == 0) return 1;
    return 0;
}

static int has_non_terminal(const char *s) {
    for (int i = 0; s[i]; i++)
        if (is_non_terminal(s[i])) return 1;
    return 0;
}

static int terminal_len(const char *s) {
    int len = 0;
    for (int i = 0; s[i]; i++)
        if (!is_non_terminal(s[i]) && s[i] != '#') len++;
    return len;
}

void generate_from_symbol(const Grammar *g, const char *current, int depth, int max_len, WordList *wl) {
    if (depth > MAX_DEPTH) return;
    if (terminal_len(current) > max_len) return;

    if (!has_non_terminal(current)) {
        /* Build final word (remove '#') */
        char word[MAX_WORD_LEN] = {0};
        int wi = 0;
        for (int i = 0; current[i] && wi < MAX_WORD_LEN-1; i++)
            if (current[i] != '#') word[wi++] = current[i];
        word[wi] = '\0';
        if (!word_exists(wl, word) && wl->count < MAX_WORDS) {
            strcpy(wl->words[wl->count++], word);
        }
        return;
    }

    /* Find leftmost non-terminal */
    int nt_pos = -1;
    for (int i = 0; current[i]; i++) {
        if (is_non_terminal(current[i])) { nt_pos = i; break; }
    }
    if (nt_pos == -1) return;
    char nt = current[nt_pos];

    /* Find productions for this NT */
    for (int i = 0; i < g->prod_count; i++) {
        if (g->productions[i].lhs != nt) continue;
        for (int j = 0; j < g->productions[i].count; j++) {
            const char *rhs = g->productions[i].rhs[j];
            /* Build new string: current[0..nt_pos-1] + rhs + current[nt_pos+1..] */
            char next[MAX_WORD_LEN * 2] = {0};
            int ni = 0;
            for (int k = 0; k < nt_pos && ni < (int)sizeof(next)-2; k++)
                next[ni++] = current[k];
            if (strcmp(rhs, "#") != 0) {
                for (int k = 0; rhs[k] && ni < (int)sizeof(next)-2; k++)
                    next[ni++] = rhs[k];
            }
            for (int k = nt_pos+1; current[k] && ni < (int)sizeof(next)-2; k++)
                next[ni++] = current[k];
            next[ni] = '\0';
            generate_from_symbol(g, next, depth+1, max_len, wl);
        }
    }
}

void generate_words(const Grammar *g, int max_length, WordList *wl) {
    wl->count = 0;
    char start[4] = {g->start_symbol, '\0'};
    generate_from_symbol(g, start, 0, max_length, wl);
}
