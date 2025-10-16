/*--------------------------------------------------------------------------------------------------------------------*/
/* SPDX-License-Identifier: MIT */
/*--------------------------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "expr.h"
/*--------------------------------------------------------------------------------------------------------------------*/
int main(int argc, char* argv[]) {
    // BASIC interpreter with line numbers, variables and functions
    char line[1024];
    printf("G2BASIC Interpreter with line numbers. Ctrl-D/Ctrl-Z to exit.\n");
    printf("Examples:\n");
    printf("  Immediate: 'x = 5', 'y = sin(x)', 'IF x > 3 THEN PRINT x'\n");
    printf("  Program: '10 x = 5', '20 IF x > 3 THEN 50', '30 PRINT \"small\"', '40 GOTO 60', '50 PRINT \"big\"', '60 END'\n");

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
                // Check if it was a PRINT, GOTO, or IF statement (don't show result)
                {
                    const char* p = line;
                    while (isspace((unsigned char)*p)) p++;
                    if ((strncmp(p, "PRINT", 5) != 0 || 
                         !(isspace((unsigned char)p[5]) || p[5] == '\0')) &&
                        (strncmp(p, "GOTO", 4) != 0 || 
                         !(isspace((unsigned char)p[4]) || p[4] == '\0')) &&
                        (strncmp(p, "IF", 2) != 0 || 
                         !(isspace((unsigned char)p[2]) || p[2] == '\0'))) {
                        // Not a PRINT, GOTO, or IF statement, show the result
                        printf("= %.*g\n", 15, val);
                    } else if (strncmp(p, "GOTO", 4) == 0) {
                        // GOTO in immediate mode doesn't make sense
                        printf("Warning: GOTO in immediate mode has no effect\n");
                    }
                }
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