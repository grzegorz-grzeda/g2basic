/*--------------------------------------------------------------------------------------------------------------------*/
/* SPDX-License-Identifier: MIT */
/*--------------------------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "expr.h"
/*--------------------------------------------------------------------------------------------------------------------*/
int main(int argc, char* argv[]) {
    // Simple REPL with variable support
    char line[1024];
    printf("Simple expression REPL with variables and functions. Ctrl-D/Ctrl-Z to exit.\n");
    printf("Examples: 'x = 5', 'y = sin(x)', 'max(x, y, 10)', 'sqrt(x*x + y*y)'\n");
    while (1) {
        printf("> ");
        if (!fgets(line, sizeof(line), stdin))
            break;
        line[strcspn(line, "\r\n")] = 0;

        // Skip empty lines
        if (strlen(line) == 0) {
            continue;
        }

        double val;
        if (expr_eval(line, &val) == 0) {
            printf("= %.*g\n", 15, val);
        } else {
            fprintf(stderr, "Error\n");
        }
    }

    return 0;
}
/*--------------------------------------------------------------------------------------------------------------------*/