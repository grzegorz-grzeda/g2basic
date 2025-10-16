/*--------------------------------------------------------------------------------------------------------------------*/
/* SPDX-License-Identifier: MIT */
/*--------------------------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "expr.h"
/*--------------------------------------------------------------------------------------------------------------------*/
int main(int argc, char* argv[]) {
    // BASIC interpreter with line numbers, variables and functions
    char line[1024];
    printf("G2BASIC Interpreter with line numbers. Ctrl-D/Ctrl-Z to exit.\n");
    printf("Examples:\n");
    printf("  Immediate: 'x = 5', 'y = sin(x)', 'max(x, y, 10)'\n");
    printf("  Program: '10 x = 5', '20 print x', 'LIST', 'RUN', 'NEW'\n");

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
        int result = expr_parse_line(line, &val);

        switch (result) {
            case 0:  // Immediate evaluation success
                printf("= %.*g\n", 15, val);
                break;
            case 1:  // Line deleted
                printf("Line %.*g deleted\n", 0, val);
                break;
            case 2:  // Line stored
                printf("Line %.*g stored\n", 0, val);
                break;
            case 3:  // Special command executed
                // Already handled, no output needed
                break;
            case -1:  // Error
                fprintf(stderr, "Error\n");
                break;
        }
    }

    return 0;
}
/*--------------------------------------------------------------------------------------------------------------------*/