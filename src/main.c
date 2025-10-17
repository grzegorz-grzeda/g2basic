/*--------------------------------------------------------------------------------------------------------------------*/
/* SPDX-License-Identifier: MIT */
/*--------------------------------------------------------------------------------------------------------------------*/
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "expr.h"
/*--------------------------------------------------------------------------------------------------------------------*/
#define MAX_LINE_LENGTH 256
/*--------------------------------------------------------------------------------------------------------------------*/
int main(int argc, char* argv[]) {
    char line[MAX_LINE_LENGTH];
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
        expr_parse_line(line, &val);
    }

    return 0;
}
/*--------------------------------------------------------------------------------------------------------------------*/