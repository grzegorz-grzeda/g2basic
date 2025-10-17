/*--------------------------------------------------------------------------------------------------------------------*/
/* SPDX-License-Identifier: MIT */
/*--------------------------------------------------------------------------------------------------------------------*/
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "g2basic.h"
/*--------------------------------------------------------------------------------------------------------------------*/
#define MAX_LINE_LENGTH 256
/*--------------------------------------------------------------------------------------------------------------------*/
/* Print function for BASIC output */
static void basic_print(const char* str) {
    printf("%s", str);
    fflush(stdout);  // Ensure immediate output
}
/*--------------------------------------------------------------------------------------------------------------------*/
int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    char line[MAX_LINE_LENGTH];

    g2basic_init(basic_print);

    printf(
        "G2BASIC Interpreter with line numbers. Ctrl-C/Ctrl-D/Ctrl-Z to "
        "exit.\n\n");

    while (1) {
        printf("> ");
        if (!fgets(line, sizeof(line), stdin))
            break;
        line[strcspn(line, "\r\n")] = 0;

        if (strlen(line) == 0) {
            continue;
        }

        double val;
        const char* error = NULL;
        g2basic_parse(line, &val, &error);
        if (error) {
            printf("Error: %s\n", error);
        }
    }

    return 0;
}
/*--------------------------------------------------------------------------------------------------------------------*/