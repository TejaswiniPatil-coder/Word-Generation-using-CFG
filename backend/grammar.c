#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/grammar.h"

int is_non_terminal(char c) {
    return isupper(c);
}

int is_terminal(char c) {
    return (islower(c) || isdigit(c) || c == '#');
}

void identify_symbols(Grammar *g) {
    g->terminal_count = 0;
    g->non_terminal_count = 0;
    for (int i = 0; i < g->prod_count; i++) {
        char lhs = g->productions[i].lhs;
        int found = 0;
        for (int k = 0; k < g->non_terminal_count; k++)
            if (g->non_terminals[k] == lhs) { found = 1; break; }
        if (!found) g->non_terminals[g->non_terminal_count++] = lhs;

        for (int j = 0; j < g->productions[i].count; j++) {
            const char *rhs = g->productions[i].rhs[j];
            for (int r = 0; r < (int)strlen(rhs); r++) {
                char c = rhs[r];
                if (is_terminal(c) && c != '#') {
                    int tf = 0;
                    for (int k = 0; k < g->terminal_count; k++)
                        if (g->terminals[k] == c) { tf = 1; break; }
                    if (!tf) g->terminals[g->terminal_count++] = c;
                }
            }
        }
    }
}

/* Parse a grammar line: "S->aSb|#" or "S → aSb | ε" */
static int parse_line(Grammar *g, const char *line) {
    char buf[256];
    strncpy(buf, line, sizeof(buf)-1);
    buf[sizeof(buf)-1] = '\0';

    /* strip trailing newline/spaces */
    int len = strlen(buf);
    while (len > 0 && (buf[len-1] == '\n' || buf[len-1] == '\r' || buf[len-1] == ' '))
        buf[--len] = '\0';
    if (len == 0) return 0;

    /* find '->' or '→' */
    char *arrow = strstr(buf, "->");
    int arrow_len = 2;
    if (!arrow) { arrow = strstr(buf, "→"); arrow_len = 3; }
    if (!arrow) return 0;

    char lhs_buf[8];
    int lhs_len = arrow - buf;
    strncpy(lhs_buf, buf, lhs_len);
    lhs_buf[lhs_len] = '\0';
    /* trim lhs */
    char lhs = 0;
    for (int i = 0; i < lhs_len; i++)
        if (!isspace((unsigned char)lhs_buf[i])) { lhs = lhs_buf[i]; break; }
    if (!lhs || !is_non_terminal(lhs)) return 0;

    /* find existing production for this lhs or create new */
    int idx = -1;
    for (int i = 0; i < g->prod_count; i++)
        if (g->productions[i].lhs == lhs) { idx = i; break; }
    if (idx == -1) {
        idx = g->prod_count++;
        g->productions[idx].lhs = lhs;
        g->productions[idx].count = 0;
    }

    /* rhs: split by '|' */
    char *rhs_start = arrow + arrow_len;
    char rhs_copy[200];
    strncpy(rhs_copy, rhs_start, sizeof(rhs_copy)-1);
    rhs_copy[sizeof(rhs_copy)-1] = '\0';

    char *token = strtok(rhs_copy, "|");
    while (token) {
        /* trim spaces */
        while (*token == ' ') token++;
        char *end = token + strlen(token) - 1;
        while (end > token && *token && *end == ' ') *end-- = '\0';

        /* replace ε with # (empty) */
        char rhs_clean[MAX_RHS_LEN] = {0};
        int ri = 0;
        for (int i = 0; token[i] && ri < MAX_RHS_LEN-1; i++) {
            unsigned char c = token[i];
            /* skip UTF-8 multi-byte for ε (0xCE 0xB5) */
            if (c == 0xCE && (unsigned char)token[i+1] == 0xB5) {
                rhs_clean[ri++] = '#';
                i++;
            } else if (c == 0xCF && (unsigned char)token[i+1] == 0xB5) {
                rhs_clean[ri++] = '#';
                i++;
            } else if (c > 127) {
                /* skip other multibyte */
            } else {
                rhs_clean[ri++] = c;
            }
        }
        rhs_clean[ri] = '\0';
        if (strlen(rhs_clean) == 0) strcpy(rhs_clean, "#");

        int pc = g->productions[idx].count;
        if (pc < MAX_RHS) {
            strncpy(g->productions[idx].rhs[pc], rhs_clean, MAX_RHS_LEN-1);
            g->productions[idx].count++;
        }
        token = strtok(NULL, "|");
    }
    return 1;
}

int parse_grammar_string(Grammar *g, const char *input) {
    g->prod_count = 0;
    char buf[2048];
    strncpy(buf, input, sizeof(buf)-1);
    buf[sizeof(buf)-1] = '\0';

    char *line = strtok(buf, "\n");
    while (line) {
        parse_line(g, line);
        line = strtok(NULL, "\n");
    }

    if (g->prod_count == 0) return 0;
    g->start_symbol = g->productions[0].lhs;
    identify_symbols(g);
    return 1;
}

int load_grammar(Grammar *g, const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) return 0;
    g->prod_count = 0;
    char line[256];
    while (fgets(line, sizeof(line), f))
        parse_line(g, line);
    fclose(f);
    if (g->prod_count == 0) return 0;
    g->start_symbol = g->productions[0].lhs;
    identify_symbols(g);
    return 1;
}

void print_grammar(const Grammar *g) {
    printf("Grammar (start: %c):\n", g->start_symbol);
    for (int i = 0; i < g->prod_count; i++) {
        printf("  %c ->", g->productions[i].lhs);
        for (int j = 0; j < g->productions[i].count; j++) {
            const char *r = g->productions[i].rhs[j];
            if (strcmp(r, "#") == 0) printf(" ε");
            else printf(" %s", r);
            if (j < g->productions[i].count-1) printf(" |");
        }
        printf("\n");
    }
}
