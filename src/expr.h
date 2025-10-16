/*--------------------------------------------------------------------------------------------------------------------*/
/* SPDX-License-Identifier: MIT */
/*--------------------------------------------------------------------------------------------------------------------*/
#ifndef EXPR_H
#define EXPR_H
/*--------------------------------------------------------------------------------------------------------------------*/
/* Evaluate an expression or assignment and return the result 
   Supports expressions (e.g., "2 + 3 * x"), assignments (e.g., "x = 5 + 3"), 
   and function calls (e.g., "sin(x)", "max(a, b, c)") */
int expr_eval(const char* expr, double* result);

/* Register a custom function with the expression evaluator
   name: function name (must be a valid identifier)
   arg_count: number of arguments (-1 for variadic functions)
   func_ptr: pointer to function that takes (double[], int) and returns double
   Returns 0 on success, -1 on error */
int expr_register_function(const char* name, int arg_count, double (*func_ptr)(double[], int));
/*--------------------------------------------------------------------------------------------------------------------*/
#endif  // EXPR_H