/**
 * @file g2basic.h
 * @brief Public API of the G2Basic interpreter.
 *
 * Include this header to embed the interpreter in a host program. The language
 * itself is described in `docs/language.md`, and integration in
 * `docs/embedding.md`.
 *
 * @author Grzegorz Grzęda
 * @copyright SPDX-License-Identifier: MIT
 */

/*--------------------------------------------------------------------------------------------------------------------*/
/* SPDX-License-Identifier: MIT */
/*--------------------------------------------------------------------------------------------------------------------*/
#ifndef G2BASIC_H
#define G2BASIC_H
/*--------------------------------------------------------------------------------------------------------------------*/
/**
 * @defgroup api Embedding API
 * @brief Functions a host program calls to run BASIC.
 *
 * The interpreter keeps a single global state: variables, registered functions,
 * the stored program, and the FOR and GOSUB stacks. It is not reentrant or
 * thread-safe. Call it from one execution context at a time, never from an
 * interrupt handler. State is allocated with `calloc` and released only by the
 * next g2basic_init() call.
 * @{
 */
/*--------------------------------------------------------------------------------------------------------------------*/
/**
 * @defgroup api_setup Setup and output
 * @brief Resetting the interpreter and routing its output.
 * @{
 */
/**
 * @brief Reset the interpreter and set the text output callback.
 *
 * Frees all variables, registered functions (custom ones included), and stored
 * program lines, and clears the FOR and GOSUB stacks. It then registers the
 * built-in functions selected at build time (see @ref math_backends) and resets
 * the number formatter set by g2basic_set_number_output().
 *
 * Call it once before any other function, and again to start over. Custom
 * functions must be registered again after each call.
 *
 * @param print_func Receives all interpreter output as NUL-terminated strings:
 *                   PRINT values, separators, and newlines, LIST output, and
 *                   RUN error messages. Each string is valid only during the
 *                   call. `NULL` disables output.
 */
void g2basic_init(void (*print_func)(const char* str));
/**
 * @brief Replace the formatter for numbers printed by PRINT.
 *
 * By default each value is formatted with 15 significant digits (`%.15g`) and
 * passed to the text callback. With a formatter set, PRINT passes each value to
 * @p print_number instead; separators and newlines still go to the text
 * callback. The formatter is not called while the text callback is `NULL`.
 * g2basic_init() resets it.
 *
 * @param print_number Number formatter, or `NULL` to restore the default.
 */
void g2basic_set_number_output(void (*print_number)(double value));
/** @} */
/*--------------------------------------------------------------------------------------------------------------------*/
/**
 * @defgroup api_functions Custom functions
 * @brief Making C functions callable from BASIC expressions.
 * @{
 */
/**
 * @brief Make a C function callable from BASIC expressions.
 *
 * BASIC code calls it as `name(arg, ...)`. The interpreter checks the argument
 * count before calling. It passes at most 8 arguments; more is the error
 * `too many function arguments`. There is no error channel for the result: a
 * NaN result is an ordinary value. Assigning NaN to a variable makes that
 * variable read as undefined.
 *
 * @param name      Name used in BASIC; the string is copied. Names are
 *                  case-sensitive, and the built-in functions are lowercase
 *                  (`sqrt`). The name must be a valid identifier (a letter or
 *                  underscore, then letters, digits, or underscores) to be
 *                  callable; this is not validated. Must not be `NULL`.
 * @param arg_count Required number of arguments from 0 to 8, or -1 to accept
 *                  any number up to 8. A call with a different count fails
 *                  with the error "function 'name' expects N arguments,
 *                  got M".
 * @param func_ptr  Implementation. It receives the evaluated arguments and
 *                  their count and returns the result. Must not be `NULL`;
 *                  this is not checked.
 *
 * @retval 0  The function is registered.
 * @retval -1 The name is already registered (built-in functions included), or
 *            memory allocation failed. An existing function is never replaced.
 *
 * @code
 * static double square(double args[], int count) {
 *     (void)count;  // Checked by the interpreter: always 1.
 *     return args[0] * args[0];
 * }
 *
 * g2basic_register_function("square", 1, square);
 * // BASIC: PRINT square(5)  ->  25
 * @endcode
 */
int g2basic_register_function(const char* name,
                              int arg_count,
                              double (*func_ptr)(double[], int));
/** @} */
/*--------------------------------------------------------------------------------------------------------------------*/
/**
 * @defgroup api_execution Execution
 * @brief Entering program lines, running statements, and running programs.
 * @{
 */
/**
 * @brief Process one line of input.
 *
 * The line is classified in this order:
 *
 * 1. `LIST`, `RUN`, or `NEW` as the first word, in any letter case, runs that
 *    command. Any text after the command is ignored.
 * 2. A line starting with a digit begins with a line number from 0 to 65535.
 *    With nothing after it, the numbered line is deleted. Otherwise the rest
 *    of the line is stored, replacing any line with that number. Stored text
 *    is not checked until it runs.
 * 3. Anything else is executed immediately as one statement.
 *
 * Because of rule 2, an immediate expression cannot start with a digit:
 * `2 + 3` stores line 2. Write `PRINT 2 + 3` or `(2 + 3)` instead.
 *
 * `RUN` executes the stored lines in line-number order. A runtime error stops
 * the program and is reported through the text callback as
 * `Error in line N: message`; g2basic_parse() still returns 3.
 *
 * @param input      NUL-terminated line without a line terminator. It is not
 *                   modified or retained.
 * @param[out] result Must not be `NULL`. Receives the statement's value for
 *                   return value 0 (the value of an expression or assignment,
 *                   otherwise 0), the line number for 1 and 2, and 0 for 3.
 *                   Unchanged for -1.
 * @param[out] error Must not be `NULL`. Set only on failure, so initialize the
 *                   pointed-to value to `NULL` before the call. The message is
 *                   static and must not be freed. Some messages are rewritten
 *                   by the next call. It is not set when the line number is
 *                   out of range.
 *
 * @retval 0  An immediate statement ran successfully.
 * @retval 1  A program line was deleted, or did not exist.
 * @retval 2  A program line was stored.
 * @retval 3  `LIST`, `RUN`, or `NEW` ran.
 * @retval -1 The immediate statement failed, or the line number is outside
 *            0 to 65535.
 *
 * @code
 * double result;
 * const char* error = NULL;
 *
 * g2basic_parse("10 FOR I = 1 TO 3", &result, &error);  // 2: stored
 * g2basic_parse("20 PRINT I", &result, &error);         // 2: stored
 * g2basic_parse("30 NEXT I", &result, &error);          // 2: stored
 * g2basic_parse("RUN", &result, &error);                // 3: prints 1, 2, 3
 *
 * if (g2basic_parse("y = undefined_name", &result, &error) < 0) {
 *     printf("Error: %s\n", error);  // undefined variable 'undefined_name'
 * }
 * @endcode
 */
int g2basic_parse(const char* input, double* result, const char** error);
/** @} */
/*--------------------------------------------------------------------------------------------------------------------*/
/** @} */
/*--------------------------------------------------------------------------------------------------------------------*/
#endif  // G2BASIC_H
