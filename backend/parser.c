#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/grammar.h"
#include "../include/parser.h"
#include "../include/tree.h"

static int has_non_terminal(const char *s) {
    for (int i = 0; s[i]; i++)
        if (is_non_terminal(s[i])) return 1;
    return 0;
}

static void add_step(ParseResult *result, const char *s) {
    if (result->count >= MAX_DERIV_STEPS) return;
    int di = 0;
    for (int i = 0; s[i] && di < MAX_STEP_LEN-3; i++) {
        if (s[i] == '#') {
            result->steps[result->count][di++] = (char)0xCE;
            result->steps[result->count][di++] = (char)0xB5;
        } else {
            result->steps[result->count][di++] = s[i];
        }
    }
    result->steps[result->count][di] = '\0';
    result->count++;
}

/* Check if string (ignoring #) equals target */
static int matches_target(const char *s, const char *target) {
    char resolved[MAX_STEP_LEN] = {0};
    int ri = 0;
    for (int i = 0; s[i] && ri < MAX_STEP_LEN-1; i++)
        if (s[i] != '#') resolved[ri++] = s[i];
    resolved[ri] = '\0';
    return strcmp(resolved, target) == 0;
}

/* Derivation-only (no tree), returns 1 if target reachable from current */
static int derive_r(const Grammar *g, const char *current, const char *target,
                    ParseResult *result, int depth) {
    if (depth > 40) return 0;

    if (!has_non_terminal(current))
        return matches_target(current, target);

    int term_count = 0;
    for (int i = 0; current[i]; i++)
        if (!is_non_terminal(current[i]) && current[i] != '#') term_count++;
    if (term_count > (int)strlen(target)) return 0;

    int nt_pos = -1;
    for (int i = 0; current[i]; i++)
        if (is_non_terminal(current[i])) { nt_pos = i; break; }
    if (nt_pos == -1) return 0;
    char nt = current[nt_pos];

    for (int i = 0; i < g->prod_count; i++) {
        if (g->productions[i].lhs != nt) continue;
        for (int j = 0; j < g->productions[i].count; j++) {
            const char *rhs = g->productions[i].rhs[j];
            char next[MAX_STEP_LEN * 2] = {0};
            int ni = 0;
            for (int k = 0; k < nt_pos && ni < (int)sizeof(next)-2; k++)
                next[ni++] = current[k];
            if (strcmp(rhs, "#") != 0) {
                for (int k = 0; rhs[k] && ni < (int)sizeof(next)-2; k++)
                    next[ni++] = rhs[k];
            } else {
                next[ni++] = '#';
            }
            for (int k = nt_pos+1; current[k] && ni < (int)sizeof(next)-2; k++)
                next[ni++] = current[k];
            next[ni] = '\0';

            int tc2 = 0;
            for (int k = 0; next[k]; k++)
                if (!is_non_terminal(next[k]) && next[k] != '#') tc2++;
            if (tc2 > (int)strlen(target)) continue;

            int saved = result->count;
            add_step(result, next);

            if (derive_r(g, next, target, result, depth+1)) return 1;
            result->count = saved;
        }
    }
    return 0;
}

/*
 * Build parse tree for a single non-terminal expanding to sub_target.
 * Returns the Node for 'nt' with its children filled.
 */
static Node* build_subtree(const Grammar *g, char nt, const char *sub_target, int depth) {
    if (depth > 40) return NULL;

    char sym[4] = {nt, '\0'};
    

    for (int i = 0; i < g->prod_count; i++) {
        if (g->productions[i].lhs != nt) continue;
        for (int j = 0; j < g->productions[i].count; j++) {
            const char *rhs = g->productions[i].rhs[j];

            /* Quick feasibility: does rhs produce sub_target? */
            /* Build sentential form: rhs */
            /* Check derivable */
            ParseResult tmp; memset(&tmp, 0, sizeof(tmp));
            if (!derive_r(g, rhs, sub_target, &tmp, 0)) {
                /* Also check direct match for terminal rhs */
                if (strcmp(rhs, "#") == 0 && strlen(sub_target) == 0) {
                    /* ok */
                } else if (!has_non_terminal(rhs) && strcmp(rhs,"#")!=0) {
                    if (strcmp(rhs, sub_target) != 0) continue;
                } else {
                    continue;
                }
            }

            Node *node = create_node(sym);

            if (strcmp(rhs, "#") == 0) {
                node->children[0] = create_node("#");
                node->child_count = 1;
                return node;
            }

            /* Check if rhs is all terminals */
            if (!has_non_terminal(rhs)) {
                if (strcmp(rhs, sub_target) != 0) { free_tree(node); continue; }
                for (int k = 0; rhs[k] && node->child_count < MAX_CHILDREN; k++) {
                    char cs[4] = {rhs[k], '\0'};
                    node->children[node->child_count++] = create_node(cs);
                }
                return node;
            }

            /* rhs has non-terminals — need to split sub_target among them */
            /* Collect the rhs symbols */
            int rhs_len = strlen(rhs);
            

            /* Try to match rhs symbols to sub_target */
            /* Use dynamic approach: 
               - fix terminals (they must match exactly)
               - NTs get the remaining substring
             */

            /* For simple case: one NT in rhs */
            int nt_count_in_rhs = 0;
            for (int k = 0; rhs[k]; k++)
                if (is_non_terminal(rhs[k])) nt_count_in_rhs++;

            if (nt_count_in_rhs == 1) {
                /* Find prefix/suffix terminal counts */
                int prefix = 0, suffix = 0;
                int nt_k = -1;
                for (int k = 0; rhs[k]; k++) {
                    if (is_non_terminal(rhs[k])) { nt_k = k; break; }
                    prefix++;
                }
                for (int k = rhs_len-1; k > nt_k; k--) {
                    if (!is_non_terminal(rhs[k])) suffix++;
                    else break;
                }
                /* Check prefix/suffix match target */
                if (prefix + suffix > (int)strlen(sub_target)) { free_tree(node); continue; }
                int nt_sub_len = (int)strlen(sub_target) - prefix - suffix;
                char nt_sub[MAX_STEP_LEN] = {0};
                strncpy(nt_sub, sub_target + prefix, nt_sub_len);

                /* Build children */
                for (int k = 0; rhs[k] && node->child_count < MAX_CHILDREN; k++) {
                    char cs[4] = {rhs[k], '\0'};
                    if (is_non_terminal(rhs[k])) {
                        Node *sub = build_subtree(g, rhs[k], nt_sub, depth+1);
                        if (!sub) sub = create_node(cs); /* fallback */
                        node->children[node->child_count++] = sub;
                    } else {
                        node->children[node->child_count++] = create_node(cs);
                    }
                }
                return node;
            }

            /* Multiple NTs: try split points recursively */
            /* For now handle 2-NT case or simple patterns */
            /* Build all children; for NTs try to figure out their sub-targets */
            /* Collect rhs as array of symbols */
            char syms[MAX_RHS_LEN][4];
            int sym_count = 0;
            for (int k = 0; rhs[k] && sym_count < MAX_RHS_LEN; k++) {
                syms[sym_count][0] = rhs[k];
                syms[sym_count][1] = '\0';
                sym_count++;
            }

            /* Match terminals first to get sub-target intervals */
            /* Simple greedy left-to-right: terminals consume exact chars, NTs consume remaining */
            char remaining[MAX_STEP_LEN];
            strncpy(remaining, sub_target, sizeof(remaining)-1);
            Node *children_built[MAX_RHS_LEN];
            int children_ok = 1;

            for (int k = 0; k < sym_count && children_ok; k++) {
                char sc = syms[k][0];
                if (!is_non_terminal(sc)) {
                    /* terminal: must match first char of remaining */
                    if (remaining[0] == sc) {
                        children_built[k] = create_node(syms[k]);
                        memmove(remaining, remaining+1, strlen(remaining));
                    } else {
                        children_ok = 0;
                    }
                } else {
                    /* NT: find how much of remaining it consumes */
                    /* Count terminals left in rhs after this NT */
                    int terminals_after = 0;
                    for (int m = k+1; m < sym_count; m++)
                        if (!is_non_terminal(syms[m][0])) terminals_after++;
                    int max_consume = (int)strlen(remaining) - terminals_after;
                    if (max_consume < 0) max_consume = 0;

                    /* Try each possible consumption */
                    Node *nt_node = NULL;
                    for (int consume = max_consume; consume >= 0; consume--) {
                        char nt_sub2[MAX_STEP_LEN] = {0};
                        strncpy(nt_sub2, remaining, consume);
                        ParseResult tmp2; memset(&tmp2, 0, sizeof(tmp2));
                        char nt_sf2[4] = {sc, '\0'};
                        if (derive_r(g, nt_sf2, nt_sub2, &tmp2, 0) || strlen(nt_sub2)==0) {
                            nt_node = build_subtree(g, sc, nt_sub2, depth+1);
                            if (nt_node) {
                                memmove(remaining, remaining+consume, strlen(remaining)-consume+1);
                                break;
                            }
                        }
                    }
                    if (!nt_node) { children_ok = 0; break; }
                    children_built[k] = nt_node;
                }
            }

            if (children_ok && strlen(remaining) == 0) {
                for (int k = 0; k < sym_count; k++)
                    node->children[node->child_count++] = children_built[k];
                return node;
            } else {
                if (children_ok) {
                    for (int k = 0; k < sym_count; k++) free_tree(children_built[k]);
                }
                free_tree(node);
            }
        }
    }
    return NULL;
}

int validate_string(const Grammar *g, const char *input, ParseResult *result) {
    result->count = 0;
    result->valid = 0;
    result->parse_tree = NULL;

    char start[4] = {g->start_symbol, '\0'};
    add_step(result, start);

    result->valid = derive_r(g, start, input, result, 0);

    if (result->valid) {
        result->parse_tree = build_subtree(g, g->start_symbol, input, 0);
        if (!result->parse_tree) {
            char sym[4] = {g->start_symbol, '\0'};
            result->parse_tree = create_node(sym);
        }
    }

    return result->valid;
}
