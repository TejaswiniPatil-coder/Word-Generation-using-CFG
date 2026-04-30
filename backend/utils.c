#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/grammar.h"
#include "../include/generator.h"

void save_words(const WordList *wl, const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) return;
    fprintf(f, "Generated Words (%d total):\n", wl->count);
    for (int i = 0; i < wl->count; i++) {
        if (strlen(wl->words[i]) == 0)
            fprintf(f, "ε\n");
        else
            fprintf(f, "%s\n", wl->words[i]);
    }
    fclose(f);
}

void save_derivation(const char **steps, int count, const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) return;
    fprintf(f, "Derivation Steps:\n");
    for (int i = 0; i < count; i++)
        fprintf(f, "  Step %d: %s\n", i, steps[i]);
    fclose(f);
}

void trim(char *s) {
    int len = strlen(s);
    while (len > 0 && (s[len-1] == '\n' || s[len-1] == '\r' || s[len-1] == ' '))
        s[--len] = '\0';
}
