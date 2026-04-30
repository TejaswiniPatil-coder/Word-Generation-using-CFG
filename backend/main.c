#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/grammar.h"
#include "include/generator.h"
#include "include/parser.h"
#include "include/tree.h"

static void print_banner() {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║      CFG Simulator — Word Generator & Validator      ║\n");
    printf("║         Context-Free Grammar Toolkit  v1.0           ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");
}

static void print_menu() {
    printf("\n┌─────────────────────────────────────┐\n");
    printf("│  MENU                               │\n");
    printf("│  1. Load grammar from file          │\n");
    printf("│  2. Enter grammar manually          │\n");
    printf("│  3. Display current grammar         │\n");
    printf("│  4. Generate words (up to length N) │\n");
    printf("│  5. Validate a string               │\n");
    printf("│  6. Show parse tree (ASCII)         │\n");
    printf("│  7. Export words to file            │\n");
    printf("│  0. Exit                            │\n");
    printf("└─────────────────────────────────────┘\n");
    printf("Choice: ");
}

int main(void) {
    Grammar g;
    memset(&g, 0, sizeof(g));
    int grammar_loaded = 0;
    ParseResult last_result;
    memset(&last_result, 0, sizeof(last_result));
    int last_result_valid = 0;

    print_banner();

    int choice;
    char buf[1024];

    while (1) {
        print_menu();
        if (!fgets(buf, sizeof(buf), stdin)) break;
        choice = atoi(buf);

        switch (choice) {
        case 0:
            printf("\nGoodbye!\n");
            return 0;

        case 1: {
            printf("Enter filename (default: data/grammar.txt): ");
            fgets(buf, sizeof(buf), stdin);
            int len = strlen(buf);
            while (len > 0 && (buf[len-1]=='\n'||buf[len-1]=='\r')) buf[--len]='\0';
            if (len == 0) strcpy(buf, "data/grammar.txt");
            if (load_grammar(&g, buf)) {
                printf("✔ Grammar loaded from '%s'\n", buf);
                grammar_loaded = 1;
            } else {
                printf("✘ Failed to load grammar from '%s'\n", buf);
            }
            break;
        }

        case 2: {
            printf("Enter grammar (one production per line, blank line to finish):\n");
            printf("  Format: S->aSb|# or S->aSb|epsilon\n");
            printf("  Use # for epsilon/empty string\n\n");
            char all[2048] = {0};
            while (1) {
                printf("  > ");
                fgets(buf, sizeof(buf), stdin);
                int len = strlen(buf);
                while (len > 0 && (buf[len-1]=='\n'||buf[len-1]=='\r')) buf[--len]='\0';
                if (len == 0) break;
                strncat(all, buf, sizeof(all)-strlen(all)-2);
                strcat(all, "\n");
            }
            if (parse_grammar_string(&g, all)) {
                printf("✔ Grammar parsed successfully\n");
                grammar_loaded = 1;
            } else {
                printf("✘ Failed to parse grammar\n");
            }
            break;
        }

        case 3:
            if (!grammar_loaded) { printf("No grammar loaded.\n"); break; }
            print_grammar(&g);
            break;

        case 4: {
            if (!grammar_loaded) { printf("No grammar loaded.\n"); break; }
            printf("Enter max word length: ");
            fgets(buf, sizeof(buf), stdin);
            int maxlen = atoi(buf);
            if (maxlen <= 0 || maxlen > 20) maxlen = 8;

            WordList wl;
            generate_words(&g, maxlen, &wl);

            printf("\nGenerated %d word(s) (length ≤ %d):\n", wl.count, maxlen);
            printf("─────────────────────────────\n");
            for (int i = 0; i < wl.count; i++) {
                if (strlen(wl.words[i]) == 0)
                    printf("  [%2d] ε (empty)\n", i+1);
                else
                    printf("  [%2d] %s\n", i+1, wl.words[i]);
            }
            printf("─────────────────────────────\n");

            /* Save to file */
            FILE *f = fopen("output/words.txt", "w");
            if (f) {
                fprintf(f, "Generated Words (max length %d):\n", maxlen);
                for (int i = 0; i < wl.count; i++) {
                    if (strlen(wl.words[i]) == 0) fprintf(f, "epsilon\n");
                    else fprintf(f, "%s\n", wl.words[i]);
                }
                fclose(f);
                printf("Words saved to output/words.txt\n");
            }
            break;
        }

        case 5: {
            if (!grammar_loaded) { printf("No grammar loaded.\n"); break; }
            printf("Enter string to validate (press Enter for empty string): ");
            fgets(buf, sizeof(buf), stdin);
            int len = strlen(buf);
            while (len > 0 && (buf[len-1]=='\n'||buf[len-1]=='\r')) buf[--len]='\0';

            /* Free old result tree */
            if (last_result.parse_tree) {
                free_tree(last_result.parse_tree);
                last_result.parse_tree = NULL;
            }

            int valid = validate_string(&g, buf, &last_result);
            last_result_valid = valid;

            if (valid) {
                printf("\n✔ '%s' is VALID\n", strlen(buf)==0?"ε":buf);
                printf("\nDerivation (%d steps):\n", last_result.count);
                printf("─────────────────────────────\n");
                for (int i = 0; i < last_result.count; i++) {
                    if (i == 0) printf("  S = %s\n", last_result.steps[i]);
                    else printf("  ⇒ %s\n", last_result.steps[i]);
                }
                printf("─────────────────────────────\n");

                /* Save derivation */
                FILE *f = fopen("output/derivations.txt", "w");
                if (f) {
                    fprintf(f, "String: %s\nResult: VALID\nDerivation:\n",
                            strlen(buf)==0?"epsilon":buf);
                    for (int i = 0; i < last_result.count; i++)
                        fprintf(f, "  Step %d: %s\n", i, last_result.steps[i]);
                    fclose(f);
                }
            } else {
                printf("\n✘ '%s' is INVALID (not in language)\n", strlen(buf)==0?"ε":buf);
            }
            break;
        }

        case 6: {
            if (!last_result_valid || !last_result.parse_tree) {
                printf("No valid parse result. Run validation first (option 5).\n");
                break;
            }
            printf("\nParse Tree:\n");
            printf("─────────────────────────────\n");
            int is_last[64] = {0};
            print_tree(last_result.parse_tree, 0, is_last, 0);
            printf("─────────────────────────────\n");
            break;
        }

        case 7: {
            if (!grammar_loaded) { printf("No grammar loaded.\n"); break; }
            printf("Enter max length for export: ");
            fgets(buf, sizeof(buf), stdin);
            int maxlen = atoi(buf);
            if (maxlen <= 0) maxlen = 10;
            WordList wl;
            generate_words(&g, maxlen, &wl);
            FILE *f = fopen("output/words.txt", "w");
            if (f) {
                fprintf(f, "Generated Words (max length %d, total: %d):\n", maxlen, wl.count);
                for (int i = 0; i < wl.count; i++) {
                    if (strlen(wl.words[i]) == 0) fprintf(f, "epsilon\n");
                    else fprintf(f, "%s\n", wl.words[i]);
                }
                fclose(f);
                printf("✔ Exported %d words to output/words.txt\n", wl.count);
            }
            break;
        }

        default:
            printf("Invalid choice.\n");
        }
    }
    return 0;
}
