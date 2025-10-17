/**
 * @file main.c
 * @brief G2Basic Interactive Interpreter Main Program
 *
 * This file contains the main interactive REPL (Read-Eval-Print Loop) for
 * the G2Basic interpreter. It provides a command-line interface that allows
 * users to enter BASIC language statements interactively.
 *
 * Features:
 * - Interactive command-line interface
 * - Real-time BASIC statement execution
 * - Program line entry and management
 * - Immediate expression evaluation
 * - Error reporting and handling
 *
 * The program accepts both immediate statements (executed right away) and
 * numbered program lines (stored for later execution with RUN command).
 *
 * @author G2Basic Development Team
 * @version 0.0.1
 * @date 2025
 * @copyright SPDX-License-Identifier: MIT
 */

/*--------------------------------------------------------------------------------------------------------------------*/
/* SPDX-License-Identifier: MIT */
/*--------------------------------------------------------------------------------------------------------------------*/
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "g2basic.h"

/*--------------------------------------------------------------------------------------------------------------------*/

/** @brief Maximum length of input lines from user */
#define MAX_LINE_LENGTH 256

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @brief Print function for BASIC program output
 *
 * This function is passed to the G2Basic interpreter to handle all output
 * from BASIC PRINT statements. It writes output to stdout and ensures
 * immediate display by flushing the output buffer.
 *
 * @param str The string to print (null-terminated)
 *
 * @note This function is called by the interpreter for all PRINT statements
 * @note Output is immediately flushed to ensure real-time display
 */
static void basic_print(const char* str) {
    printf("%s", str);
    fflush(stdout);  // Ensure immediate output
}

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @brief Main program entry point
 *
 * Initializes the G2Basic interpreter and runs the interactive REPL
 * (Read-Eval-Print Loop). The program provides a command-line interface where
 * users can:
 *
 * - Enter immediate BASIC statements for execution
 * - Store numbered program lines for later execution
 * - Execute stored programs with the RUN command
 * - Manage and edit BASIC programs interactively
 *
 * The loop continues until the user terminates the program with Ctrl-C, Ctrl-D,
 * or Ctrl-Z. All errors are reported with descriptive messages to help users
 * debug their BASIC programs.
 *
 * @param argc Command line argument count (unused)
 * @param argv Command line argument vector (unused)
 * @return 0 on successful termination
 *
 * @note The program uses stdin for input and stdout for output
 * @note Lines longer than MAX_LINE_LENGTH are truncated
 * @note Empty lines are ignored and do not generate errors
 */
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