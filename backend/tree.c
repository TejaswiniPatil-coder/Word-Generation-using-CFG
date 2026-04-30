#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/tree.h"

Node* create_node(const char *symbol) {
    Node *n = (Node*)malloc(sizeof(Node));
    strncpy(n->symbol, symbol, sizeof(n->symbol)-1);
    n->symbol[sizeof(n->symbol)-1] = '\0';
    n->child_count = 0;
    for (int i = 0; i < MAX_CHILDREN; i++) n->children[i] = NULL;
    return n;
}

void free_tree(Node *root) {
    if (!root) return;
    for (int i = 0; i < root->child_count; i++)
        free_tree(root->children[i]);
    free(root);
}

void print_tree(Node *root, int depth, int *is_last, int level) {
    if (!root) return;
    for (int i = 0; i < depth-1; i++)
        printf(is_last[i] ? "    " : "│   ");
    if (depth > 0)
        printf(is_last[depth-1] ? "└── " : "├── ");
    const char *sym = root->symbol;
    if (strcmp(sym, "#") == 0) printf("ε\n");
    else printf("%s\n", sym);

    for (int i = 0; i < root->child_count; i++) {
        is_last[depth] = (i == root->child_count - 1);
        print_tree(root->children[i], depth+1, is_last, level+1);
    }
}

void tree_to_json(Node *root, char *buf, int *pos) {
    if (!root) return;
    const char *sym = root->symbol;
    char display[64];
    if (strcmp(sym, "#") == 0) strcpy(display, "ε");
    else strncpy(display, sym, sizeof(display)-1);

    *pos += sprintf(buf + *pos, "{\"name\":\"%s\",\"children\":[", display);
    for (int i = 0; i < root->child_count; i++) {
        if (i > 0) buf[(*pos)++] = ',';
        tree_to_json(root->children[i], buf, pos);
    }
    *pos += sprintf(buf + *pos, "]}");
}
