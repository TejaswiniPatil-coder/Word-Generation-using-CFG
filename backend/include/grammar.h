#ifndef GRAMMAR_H
#define GRAMMAR_H

#define MAX_PRODUCTIONS 50
#define MAX_RHS 20
#define MAX_RHS_LEN 30
#define MAX_SYMBOLS 26

typedef struct {
    char lhs;
    char rhs[MAX_RHS][MAX_RHS_LEN];
    int count;
} Production;

typedef struct {
    Production productions[MAX_PRODUCTIONS];
    int prod_count;
    char start_symbol;
    char terminals[MAX_SYMBOLS];
    int terminal_count;
    char non_terminals[MAX_SYMBOLS];
    int non_terminal_count;
} Grammar;

int load_grammar(Grammar *g, const char *filename);
int parse_grammar_string(Grammar *g, const char *input);
void print_grammar(const Grammar *g);
int is_terminal(char c);
int is_non_terminal(char c);
void identify_symbols(Grammar *g);

#endif
