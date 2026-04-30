#ifndef PARSER_H
#define PARSER_H

#include "grammar.h"
#include "tree.h"

#define MAX_DERIV_STEPS 100
#define MAX_STEP_LEN 200

typedef struct {
    char steps[MAX_DERIV_STEPS][MAX_STEP_LEN];
    int count;
    int valid;
    Node *parse_tree;
} ParseResult;

int validate_string(const Grammar *g, const char *input, ParseResult *result);
int derive(const Grammar *g, const char *current, const char *target,
           ParseResult *result, Node *node, int depth);

#endif
