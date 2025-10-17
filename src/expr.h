/*--------------------------------------------------------------------------------------------------------------------*/
/* SPDX-License-Identifier: MIT */
/*--------------------------------------------------------------------------------------------------------------------*/
#ifndef EXPR_H
#define EXPR_H
/*--------------------------------------------------------------------------------------------------------------------*/
/* Initialize the expression evaluator system
   Clears all variables, program lines, and resets the interpreter state
   Registers built-in math functions
   print_func: function pointer for output (pass NULL to disable output) */
void expr_init(void (*print_func)(const char* str));

/* Evaluate an expression or assignment and return the result
   Supports expressions (e.g., "2 + 3 * x"), assignments (e.g., "x = 5 + 3"),
   and function calls (e.g., "sin(x)", "max(a, b, c)") */
int expr_eval(const char* expr, double* result, const char** error);

/* Register a custom function with the expression evaluator
   name: function name (must be a valid identifier)
   arg_count: number of arguments (-1 for variadic functions)
   func_ptr: pointer to function that takes (double[], int) and returns double
   Returns 0 on success, -1 on error */
int expr_register_function(const char* name,
                           int arg_count,
                           double (*func_ptr)(double[], int));

/* Parse a line that may contain a line number for BASIC program entry
   input: line to parse (may start with line number)
   result: pointer to store result value
   Returns: 0 = immediate evaluation success, 1 = line deleted, 2 = line stored,
   -1 = error */
int expr_parse_line(const char* input, double* result, const char** error);
/*--------------------------------------------------------------------------------------------------------------------*/
#endif  // EXPR_H