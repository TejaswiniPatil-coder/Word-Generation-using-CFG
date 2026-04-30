#ifndef TREE_H
#define TREE_H

#define MAX_CHILDREN 10
#define MAX_TREE_NODES 500

typedef struct Node {
    char symbol[32];
    struct Node *children[MAX_CHILDREN];
    int child_count;
} Node;

Node* create_node(const char *symbol);
void free_tree(Node *root);
void print_tree(Node *root, int depth, int *is_last, int level);
void tree_to_json(Node *root, char *buf, int *pos);

#endif
