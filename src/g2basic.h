/**
 * @file g2basic.h
 * @brief G2Basic BASIC Language Interpreter Header
 *
 * This file contains the public API for the G2Basic interpreter, a lightweight
 * BASIC language interpreter designed for microcontrollers and embedded
 * systems.
 *
 * The interpreter supports:
 * - Dynamic memory management with linked lists
 * - Variables and mathematical expressions
 * - Control flow statements (IF/THEN, FOR/NEXT, GOTO, GOSUB/RETURN)
 * - Built-in mathematical functions
 * - Line-based BASIC program execution
 * - Configurable output system
 *
 * @author Grzegorz Grzęda
 * @version 0.0.1
 * @date 2025
 * @copyright SPDX-License-Identifier: MIT
 */

/*--------------------------------------------------------------------------------------------------------------------*/
/* SPDX-License-Identifier: MIT */
/*--------------------------------------------------------------------------------------------------------------------*/
#ifndef G2BASIC_H
#define G2BASIC_H
/*--------------------------------------------------------------------------------------------------------------------*/
/**
 * @brief Initialize the G2Basic interpreter system
 *
 * This function initializes the entire G2Basic interpreter, clearing all
 * variables, program lines, and resetting the interpreter state to a clean
 * starting condition. It also registers all built-in mathematical functions.
 *
 * The function performs the following operations:
 * - Clears all variables from memory
 * - Clears all stored program lines
 * - Resets FOR loop and GOSUB stacks
 * - Resets GOTO target and execution state
 * - Registers built-in math functions (sin, cos, tan, sqrt, abs, pow, etc.)
 * - Sets up the print function for program output
 *
 * @param print_func Function pointer for output operations. Pass NULL to
 *                   disable output. The function should accept a const char*
 *                   string and handle its display/logging appropriately.
 *
 * @note This function must be called before any other G2Basic operations.
 * @note The print function will be called for all PRINT statements in BASIC
 * programs.
 *
 * @see g2basic_parse()
 * @see g2basic_register_function()
 *
 * @since 0.0.1
 */
void g2basic_init(void (*print_func)(const char* str));
/*--------------------------------------------------------------------------------------------------------------------*/
/**
 * @brief Register a custom function with the expression evaluator
 *
 * This function allows registration of custom functions that can be called
 * from within BASIC expressions and programs. Functions can have a fixed
 * number of arguments or be variadic.
 *
 * The registered function will be available for use in BASIC expressions
 * using standard function call syntax: FUNCTION_NAME(arg1, arg2, ...)
 *
 * @param name The function name as it will appear in BASIC code. Must be a
 *             valid identifier (alphanumeric + underscore, starting with
 * letter). The name is case-sensitive.
 *
 * @param arg_count Number of arguments the function expects:
 *                  - Positive integer: Fixed number of arguments
 *                  - -1: Variadic function (variable number of arguments)
 *
 * @param func_ptr Pointer to the C function implementing the functionality.
 *                 The function must have the signature:
 *                 double function_name(double args[], int count)
 *                 - args[]: Array of argument values
 *                 - count: Number of arguments passed
 *                 - Returns: Double precision result value
 *
 * @return 0 on success, -1 on error (invalid name, memory allocation failure,
 * etc.)
 *
 * @note Function names must be unique. Registering a function with an existing
 *       name will replace the previous function.
 * @note Variadic functions should validate the argument count internally
 * @note Functions should return NAN for invalid arguments or error conditions
 *
 * @warning The function pointer must remain valid for the lifetime of the
 * interpreter
 *
 * @see g2basic_init()
 *
 * @since 0.0.1
 *
 * @code
 * // Example: Register a simple square function
 * double my_square(double args[], int count) {
 *     if (count != 1) return NAN;  // Expect exactly 1 argument
 *     return args[0] * args[0];
 * }
 *
 * g2basic_register_function("SQUARE", 1, my_square);
 * // Now can use: PRINT SQUARE(5)  -> outputs 25
 * @endcode
 */
int g2basic_register_function(const char* name,
                              int arg_count,
                              double (*func_ptr)(double[], int));
/*--------------------------------------------------------------------------------------------------------------------*/
/**
 * @brief Parse and execute a BASIC language line
 *
 * This is the main entry point for processing BASIC language input. The
 * function can handle both immediate expression evaluation and program line
 * storage/execution.
 *
 * The function supports several input formats:
 * - Immediate expressions: "PRINT 2 + 3" (executed immediately)
 * - Program lines: "10 PRINT \"Hello\"" (stored for later execution)
 * - Line deletion: "10" (deletes line 10 from stored program)
 * - Program execution: "RUN" (executes stored program from lowest line number)
 *
 * Program lines are automatically sorted by line number and can be executed
 * with control flow statements like GOTO, FOR/NEXT, IF/THEN, and GOSUB/RETURN.
 *
 * @param input The BASIC language line to parse. Can contain:
 *              - Line number followed by statement: "10 PRINT \"Hello\""
 *              - Just line number (deletes that line): "10"
 *              - Immediate statement: "PRINT 2 + 3"
 *              - Program control: "RUN", "LIST"
 *
 * @param result Pointer to store the result value. For statements that produce
 *               a value (expressions, calculations), the result is stored here.
 *               For statements without return values, this may be set to 0.
 *               Can be NULL if result is not needed.
 *
 * @param error Pointer to store error message string. If parsing or execution
 *              fails, this will point to a descriptive error message string.
 *              The string is statically allocated and should not be freed.
 *              Can be NULL if error information is not needed.
 *
 * @return Execution result code:
 *         - 0: Immediate evaluation success (expression calculated and
 * executed)
 *         - 1: Line deleted successfully from program
 *         - 2: Line stored successfully in program
 *         - -1: Error occurred (check *error for details)
 *
 * @note The interpreter maintains program state between calls, allowing
 *       multi-line BASIC programs to be built and executed
 * @note Line numbers must be positive integers (1-999999 typical range)
 * @note Duplicate line numbers replace existing lines
 * @note The function is thread-safe if only one thread calls it at a time
 *
 * @warning Input string is temporarily modified during parsing but restored
 * @warning Error messages are statically allocated and may be overwritten
 *          by subsequent calls
 *
 * @see g2basic_init()
 * @see g2basic_register_function()
 *
 * @since 0.0.1
 *
 * @code
 * // Example usage:
 * double result;
 * const char* error;
 *
 * // Store a program line
 * int ret = g2basic_parse("10 FOR I = 1 TO 5", &result, &error);
 * if (ret == 2) printf("Line stored\\n");
 *
 * // Add more lines...
 * g2basic_parse("20 PRINT I", &result, &error);
 * g2basic_parse("30 NEXT I", &result, &error);
 *
 * // Execute the program
 * ret = g2basic_parse("RUN", &result, &error);
 * if (ret == -1) printf("Error: %s\\n", error);
 * @endcode
 */
int g2basic_parse(const char* input, double* result, const char** error);
/*--------------------------------------------------------------------------------------------------------------------*/
#endif  // G2BASIC_H