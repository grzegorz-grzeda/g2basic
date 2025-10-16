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

typedef struct {
    char name[MAX_VAR_NAME];
    double value;
} Variable;

static Variable variables[MAX_VARIABLES];
static int num_variables = 0;

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
   factor:= NUMBER | VARIABLE | '(' expr ')' | ('+'|'-') factor   (unary)
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

    // Try to parse as variable first
    if (is_alpha_or_underscore(*p->s)) {
        return parse_variable(p);
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
