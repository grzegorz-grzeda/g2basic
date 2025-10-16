/*--------------------------------------------------------------------------------------------------------------------*/
/* SPDX-License-Identifier: MIT */
/*--------------------------------------------------------------------------------------------------------------------*/
#include "expr.h"
#include <ctype.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*--------------------------------------------------------------------------------------------------------------------*/
#define MAX_VARIABLES 100
#define MAX_VAR_NAME 32
#define MAX_FUNCTIONS 100
#define MAX_FUNC_NAME 32
#define MAX_FUNC_ARGS 8

typedef struct {
    char name[MAX_VAR_NAME];
    double value;
} Variable;

typedef struct {
    char name[MAX_FUNC_NAME];
    int arg_count;  // Number of arguments (-1 for variadic)
    double (*func_ptr)(double args[], int count);
} Function;

static Variable variables[MAX_VARIABLES];
static int num_variables = 0;
static Function functions[MAX_FUNCTIONS];
static int num_functions = 0;
static int functions_initialized = 0;

typedef struct {
    const char* start;  // beginning of the input (for position reporting)
    const char* s;      // current cursor
    const char* err;    // error message (NULL if ok)
} Parser;
/*--------------------------------------------------------------------------------------------------------------------*/
/* Forward declarations (grammar):
   statement := assignment | expr
   assignment := VARIABLE '=' expr
   expr  := term (('+'|'-') term)*
   term  := factor (('*'|'/') factor)*
   factor:= NUMBER | VARIABLE | FUNCTION_CALL | '(' expr ')' | ('+'|'-') factor   (unary)
   function_call := IDENTIFIER '(' arg_list ')'
   arg_list := expr (',' expr)*
*/
static double parse_expr(Parser* p);
static double parse_statement(Parser* p);
/*--------------------------------------------------------------------------------------------------------------------*/
static int is_alpha_or_underscore(char c) {
    return isalpha((unsigned char)c) || c == '_';
}
/*--------------------------------------------------------------------------------------------------------------------*/
static int is_alnum_or_underscore(char c) {
    return isalnum((unsigned char)c) || c == '_';
}
/*--------------------------------------------------------------------------------------------------------------------*/
static double get_variable(const char* name) {
    for (int i = 0; i < num_variables; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            return variables[i].value;
        }
    }
    return NAN;  // Variable not found
}
/*--------------------------------------------------------------------------------------------------------------------*/
static void set_variable(const char* name, double value) {
    // Check if variable already exists
    for (int i = 0; i < num_variables; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            variables[i].value = value;
            return;
        }
    }
    
    // Add new variable if there's space
    if (num_variables < MAX_VARIABLES) {
        strncpy(variables[num_variables].name, name, MAX_VAR_NAME - 1);
        variables[num_variables].name[MAX_VAR_NAME - 1] = '\0';
        variables[num_variables].value = value;
        num_variables++;
    }
}
/*--------------------------------------------------------------------------------------------------------------------*/
static Function* find_function(const char* name) {
    for (int i = 0; i < num_functions; i++) {
        if (strcmp(functions[i].name, name) == 0) {
            return &functions[i];
        }
    }
    return NULL;
}
/*--------------------------------------------------------------------------------------------------------------------*/
static int register_function(const char* name, int arg_count, double (*func_ptr)(double[], int)) {
    // Check if function already exists
    if (find_function(name) != NULL) {
        return -1;  // Function already exists
    }
    
    // Add new function if there's space
    if (num_functions < MAX_FUNCTIONS) {
        strncpy(functions[num_functions].name, name, MAX_FUNC_NAME - 1);
        functions[num_functions].name[MAX_FUNC_NAME - 1] = '\0';
        functions[num_functions].arg_count = arg_count;
        functions[num_functions].func_ptr = func_ptr;
        num_functions++;
        return 0;  // Success
    }
    
    return -1;  // No space for more functions
}
/*--------------------------------------------------------------------------------------------------------------------*/
/* Built-in math function wrappers */
static double func_sin(double args[], int count) {
    if (count != 1) return NAN;
    return sin(args[0]);
}

static double func_cos(double args[], int count) {
    if (count != 1) return NAN;
    return cos(args[0]);
}

static double func_tan(double args[], int count) {
    if (count != 1) return NAN;
    return tan(args[0]);
}

static double func_sqrt(double args[], int count) {
    if (count != 1) return NAN;
    if (args[0] < 0) return NAN;
    return sqrt(args[0]);
}

static double func_abs(double args[], int count) {
    if (count != 1) return NAN;
    return fabs(args[0]);
}

static double func_pow(double args[], int count) {
    if (count != 2) return NAN;
    return pow(args[0], args[1]);
}

static double func_log(double args[], int count) {
    if (count != 1) return NAN;
    if (args[0] <= 0) return NAN;
    return log(args[0]);
}

static double func_log10(double args[], int count) {
    if (count != 1) return NAN;
    if (args[0] <= 0) return NAN;
    return log10(args[0]);
}

static double func_exp(double args[], int count) {
    if (count != 1) return NAN;
    return exp(args[0]);
}

static double func_floor(double args[], int count) {
    if (count != 1) return NAN;
    return floor(args[0]);
}

static double func_ceil(double args[], int count) {
    if (count != 1) return NAN;
    return ceil(args[0]);
}

static double func_min(double args[], int count) {
    if (count < 1) return NAN;
    double min_val = args[0];
    for (int i = 1; i < count; i++) {
        if (args[i] < min_val) min_val = args[i];
    }
    return min_val;
}

static double func_max(double args[], int count) {
    if (count < 1) return NAN;
    double max_val = args[0];
    for (int i = 1; i < count; i++) {
        if (args[i] > max_val) max_val = args[i];
    }
    return max_val;
}
/*--------------------------------------------------------------------------------------------------------------------*/
static void init_builtin_functions(void) {
    register_function("sin", 1, func_sin);
    register_function("cos", 1, func_cos);
    register_function("tan", 1, func_tan);
    register_function("sqrt", 1, func_sqrt);
    register_function("abs", 1, func_abs);
    register_function("pow", 2, func_pow);
    register_function("log", 1, func_log);
    register_function("log10", 1, func_log10);
    register_function("exp", 1, func_exp);
    register_function("floor", 1, func_floor);
    register_function("ceil", 1, func_ceil);
    register_function("min", -1, func_min);  // Variadic
    register_function("max", -1, func_max);  // Variadic
}
/*--------------------------------------------------------------------------------------------------------------------*/
static void skip_ws(Parser* p) {
    while (isspace((unsigned char)*p->s))
        p->s++;
}
/*--------------------------------------------------------------------------------------------------------------------*/
static int accept(Parser* p, char c) {
    skip_ws(p);
    if (*p->s == c) {
        p->s++;
        return 1;
    }
    return 0;
}
/*--------------------------------------------------------------------------------------------------------------------*/
static void expect(Parser* p, char c) {
    skip_ws(p);
    if (*p->s != c) {
        static char msg[64];
        snprintf(msg, sizeof(msg), "expected '%c'", c);
        p->err = msg;
        return;
    }
    p->s++;
}
/*--------------------------------------------------------------------------------------------------------------------*/
static double parse_number(Parser* p) {
    skip_ws(p);
    const char* beg = p->s;
    // Accept optional digits, dot, digits, exponent, etc., via strtod
    char* endptr = NULL;
    double v = strtod(p->s, &endptr);
    if (endptr == p->s) {
        p->err = "expected number";
        return NAN;
    }
    p->s = endptr;
    return v;
}
/*--------------------------------------------------------------------------------------------------------------------*/
static double parse_variable(Parser* p) {
    skip_ws(p);
    
    if (!is_alpha_or_underscore(*p->s)) {
        p->err = "expected variable name";
        return NAN;
    }
    
    char var_name[MAX_VAR_NAME];
    int len = 0;
    
    // Parse variable name (starts with letter or underscore, followed by alphanumeric or underscore)
    while (len < MAX_VAR_NAME - 1 && is_alnum_or_underscore(*p->s)) {
        var_name[len++] = *p->s++;
    }
    var_name[len] = '\0';
    
    double value = get_variable(var_name);
    if (isnan(value)) {
        static char msg[64];
        snprintf(msg, sizeof(msg), "undefined variable '%s'", var_name);
        p->err = msg;
        return NAN;
    }
    
    return value;
}
/*--------------------------------------------------------------------------------------------------------------------*/
static double parse_function_call(Parser* p, const char* func_name) {
    Function* func = find_function(func_name);
    if (!func) {
        static char msg[64];
        snprintf(msg, sizeof(msg), "unknown function '%s'", func_name);
        p->err = msg;
        return NAN;
    }
    
    expect(p, '(');
    if (p->err) return NAN;
    
    double args[MAX_FUNC_ARGS];
    int arg_count = 0;
    
    // Parse arguments
    skip_ws(p);
    if (*p->s != ')') {  // Function has arguments
        do {
            if (arg_count >= MAX_FUNC_ARGS) {
                p->err = "too many function arguments";
                return NAN;
            }
            
            args[arg_count] = parse_expr(p);
            if (p->err) return NAN;
            arg_count++;
            
            skip_ws(p);
            if (*p->s == ',') {
                p->s++;  // consume comma
            } else {
                break;
            }
        } while (1);
    }
    
    expect(p, ')');
    if (p->err) return NAN;
    
    // Validate argument count
    if (func->arg_count >= 0 && arg_count != func->arg_count) {
        static char msg[64];
        snprintf(msg, sizeof(msg), "function '%s' expects %d arguments, got %d", 
                func_name, func->arg_count, arg_count);
        p->err = msg;
        return NAN;
    }
    
    // Call the function
    return func->func_ptr(args, arg_count);
}
/*--------------------------------------------------------------------------------------------------------------------*/
static double parse_factor(Parser* p) {
    skip_ws(p);
    // unary +/-
    if (*p->s == '+' || *p->s == '-') {
        int neg = (*p->s == '-');
        p->s++;
        double v = parse_factor(p);
        return neg ? -v : v;
    }

    if (accept(p, '(')) {
        double v = parse_expr(p);
        if (p->err)
            return v;
        expect(p, ')');
        return v;
    }

    // Try to parse as identifier (variable or function call)
    if (is_alpha_or_underscore(*p->s)) {
        // Parse the identifier name
        char identifier[MAX_VAR_NAME];
        int len = 0;
        const char* start = p->s;
        
        while (len < MAX_VAR_NAME - 1 && is_alnum_or_underscore(*p->s)) {
            identifier[len++] = *p->s++;
        }
        identifier[len] = '\0';
        
        skip_ws(p);
        
        // Check if it's a function call (followed by '(')
        if (*p->s == '(') {
            return parse_function_call(p, identifier);
        } else {
            // It's a variable, restore position and parse as variable
            p->s = start;
            return parse_variable(p);
        }
    }

    return parse_number(p);
}
/*--------------------------------------------------------------------------------------------------------------------*/
static double parse_term(Parser* p) {
    double v = parse_factor(p);
    while (!p->err) {
        skip_ws(p);
        if (*p->s == '*' || *p->s == '/') {
            char op = *p->s++;
            double rhs = parse_factor(p);
            if (p->err)
                return v;
            if (op == '*') {
                v *= rhs;
            } else {
                if (rhs == 0.0) {
                    p->err = "division by zero";
                    return NAN;
                }
                v /= rhs;
            }
        } else {
            break;
        }
    }
    return v;
}
/*--------------------------------------------------------------------------------------------------------------------*/
static double parse_expr(Parser* p) {
    double v = parse_term(p);
    while (!p->err) {
        skip_ws(p);
        if (*p->s == '+' || *p->s == '-') {
            char op = *p->s++;
            double rhs = parse_term(p);
            if (p->err)
                return v;
            v = (op == '+') ? (v + rhs) : (v - rhs);
        } else {
            break;
        }
    }
    return v;
}
/*--------------------------------------------------------------------------------------------------------------------*/
static double parse_statement(Parser* p) {
    skip_ws(p);
    
    // Check if this looks like an assignment (variable = ...)
    const char* saved_pos = p->s;
    
    // Try to parse a variable name
    if (is_alpha_or_underscore(*p->s)) {
        char var_name[MAX_VAR_NAME];
        int len = 0;
        
        // Parse variable name
        while (len < MAX_VAR_NAME - 1 && is_alnum_or_underscore(*p->s)) {
            var_name[len++] = *p->s++;
        }
        var_name[len] = '\0';
        
        skip_ws(p);
        
        // Check if followed by '='
        if (*p->s == '=') {
            p->s++;  // consume '='
            
            // Parse the expression on the right side
            double value = parse_expr(p);
            if (p->err) {
                return value;
            }
            
            // Set the variable
            set_variable(var_name, value);
            return value;
        }
    }
    
    // Not an assignment, restore position and parse as expression
    p->s = saved_pos;
    return parse_expr(p);
}
/*--------------------------------------------------------------------------------------------------------------------*/
int expr_eval(const char* expr, double* result) {
    // Initialize built-in functions on first call
    if (!functions_initialized) {
        init_builtin_functions();
        functions_initialized = 1;
    }
    
    Parser p = {.start = expr, .s = expr, .err = NULL};
    double v = parse_statement(&p);
    if (p.err) {
        return -1;  // Error: parsing failed
    }
    skip_ws(&p);
    if (*p.s != '\0') {
        return -1;  // Error: unexpected characters at end
    }
    *result = v;
    return 0;  // Success
}
/*--------------------------------------------------------------------------------------------------------------------*/
int expr_register_function(const char* name, int arg_count, double (*func_ptr)(double[], int)) {
    // Initialize built-in functions if not already done
    if (!functions_initialized) {
        init_builtin_functions();
        functions_initialized = 1;
    }
    
    return register_function(name, arg_count, func_ptr);
}
/*--------------------------------------------------------------------------------------------------------------------*/
