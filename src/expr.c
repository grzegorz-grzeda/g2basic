/*--------------------------------------------------------------------------------------------------------------------*/
/* SPDX-License-Identifier: MIT */
/*--------------------------------------------------------------------------------------------------------------------*/
#include "expr.h"
#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*--------------------------------------------------------------------------------------------------------------------*/
#define MAX_VAR_NAME 32
#define MAX_FUNC_ARGS 8
#define MAX_FOR_LOOPS 10
#define MAX_GOSUB_STACK 20

typedef struct Variable {
    char* name;  // Dynamically allocated variable name
    double value;
    struct Variable* next;  // Next variable in linked list
} Variable;

typedef struct Function {
    char* name;     // Dynamically allocated function name
    int arg_count;  // Number of arguments (-1 for variadic)
    double (*func_ptr)(double args[], int count);
    struct Function* next;  // Next function in linked list
} Function;

typedef struct ProgramLine {
    int line_number;
    char* text;                // Dynamically allocated program line text
    struct ProgramLine* next;  // Next program line in linked list
} ProgramLine;

typedef struct {
    char var_name[MAX_VAR_NAME];  // Loop variable name
    double start_value;           // Starting value
    double end_value;             // Ending value
    double step_value;            // Step increment (default 1)
    int for_line_number;          // Line number of FOR statement
    int next_line_number;  // Line number of NEXT statement (-1 if not found)
} ForLoop;

static Variable* variables_head = NULL;   // Head of variables linked list
static Function* functions_head = NULL;   // Head of functions linked list
static ProgramLine* program_head = NULL;  // Head of program lines linked list
static int goto_target = -1;  // Target line number for GOTO (-1 = no jump)
static ForLoop for_stack[MAX_FOR_LOOPS];  // Stack for nested FOR loops
static int for_stack_top = -1;            // Top of FOR stack (-1 = empty)
static int current_line_index = -1;       // Current executing line index
static int gosub_stack[MAX_GOSUB_STACK];  // Stack for GOSUB return addresses
                                          // (line indices)
static int gosub_stack_top = -1;          // Top of GOSUB stack (-1 = empty)

// Print function pointer for output
static void (*print_function)(const char* str) = NULL;

typedef struct {
    const char* start;  // beginning of the input (for position reporting)
    const char* s;      // current cursor
    const char* err;    // error message (NULL if ok)
} Parser;
/*--------------------------------------------------------------------------------------------------------------------*/
/* Forward declarations (grammar):
   statement := assignment | print_stmt | goto_stmt | if_stmt | for_stmt |
   next_stmt | gosub_stmt | return_stmt | expr assignment := VARIABLE '=' expr
   print_stmt := 'PRINT' expr_list goto_stmt := 'GOTO' NUMBER gosub_stmt :=
   'GOSUB' NUMBER return_stmt := 'RETURN' if_stmt := 'IF' comparison 'THEN'
   (NUMBER | statement) for_stmt := 'FOR' VARIABLE '=' expr 'TO' expr ['STEP'
   expr] next_stmt := 'NEXT' VARIABLE comparison := expr
   ('>'|'<'|'>='|'<='|'='|'<>') expr expr_list := expr (',' expr)* expr  := term
   (('+'|'-') term)* term  := factor (('*'|'/') factor)* factor:= NUMBER |
   VARIABLE | FUNCTION_CALL | '(' expr ')' | ('+'|'-') factor (unary)
   function_call := IDENTIFIER '(' arg_list ')' arg_list := expr (',' expr)*
*/
static double parse_expr(Parser* p);
static double parse_statement(Parser* p);
static const char* skip_ws(const char* p);
/*--------------------------------------------------------------------------------------------------------------------*/
/* Helper function to safely call the print function */
static void safe_print(const char* str) {
    if (print_function != NULL) {
        print_function(str);
    }
}
/*--------------------------------------------------------------------------------------------------------------------*/
/* Helper function for formatted printing */
static void safe_printf(const char* format, ...) {
    if (print_function != NULL) {
        char buffer[512];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        print_function(buffer);
    }
}
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
    Variable* current = variables_head;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current->value;
        }
        current = current->next;
    }
    return NAN;  // Variable not found
}
/*--------------------------------------------------------------------------------------------------------------------*/
static void set_variable(const char* name, double value) {
    // First try to find existing variable
    Variable* current = variables_head;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            current->value = value;
            return;
        }
        current = current->next;
    }

    // Variable not found, create new one
    Variable* new_var = (Variable*)calloc(1, sizeof(Variable));
    if (new_var == NULL) {
        return;  // Memory allocation failed
    }

    // Allocate memory for name
    new_var->name = (char*)calloc(strlen(name) + 1, sizeof(char));
    if (new_var->name == NULL) {
        free(new_var);
        return;  // Memory allocation failed
    }

    strcpy(new_var->name, name);
    new_var->value = value;
    new_var->next = variables_head;  // Insert at head
    variables_head = new_var;
}
/*--------------------------------------------------------------------------------------------------------------------*/
static void clear_all_variables(void) {
    Variable* current = variables_head;
    while (current != NULL) {
        Variable* to_delete = current;
        current = current->next;
        free(to_delete->name);  // Free the dynamically allocated name
        free(to_delete);        // Free the variable structure
    }
    variables_head = NULL;
}
/*--------------------------------------------------------------------------------------------------------------------*/
static void clear_all_functions(void) {
    Function* current = functions_head;
    while (current != NULL) {
        Function* to_delete = current;
        current = current->next;
        free(to_delete->name);  // Free the dynamically allocated name
        free(to_delete);        // Free the function structure
    }
    functions_head = NULL;
}
/*--------------------------------------------------------------------------------------------------------------------*/
static void clear_all_program_lines(void) {
    ProgramLine* current = program_head;
    while (current != NULL) {
        ProgramLine* to_delete = current;
        current = current->next;
        free(to_delete->text);  // Free the dynamically allocated text
        free(to_delete);        // Free the program line structure
    }
    program_head = NULL;
}
/*--------------------------------------------------------------------------------------------------------------------*/
static ProgramLine* find_program_line(int line_number) {
    ProgramLine* current = program_head;
    while (current != NULL) {
        if (current->line_number == line_number) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}
/*--------------------------------------------------------------------------------------------------------------------*/
static ProgramLine* find_next_program_line(int line_number) {
    ProgramLine* current = program_head;
    while (current != NULL) {
        if (current->line_number > line_number) {
            return current;  // Found first line after the given line number
        }
        current = current->next;
    }
    return NULL;  // No line found after the given line number
}
/*--------------------------------------------------------------------------------------------------------------------*/
static void insert_program_line_sorted(int line_number, const char* text) {
    // Remove existing line with same number if it exists
    ProgramLine* existing = find_program_line(line_number);
    if (existing != NULL) {
        // Remove from list
        if (existing == program_head) {
            program_head = existing->next;
        } else {
            ProgramLine* prev = program_head;
            while (prev->next != existing) {
                prev = prev->next;
            }
            prev->next = existing->next;
        }
        free(existing->text);
        free(existing);
    }

    // Create new program line
    ProgramLine* new_line = (ProgramLine*)calloc(1, sizeof(ProgramLine));
    if (new_line == NULL) {
        return;  // Memory allocation failed
    }

    new_line->line_number = line_number;
    new_line->text = (char*)calloc(strlen(text) + 1, sizeof(char));
    if (new_line->text == NULL) {
        free(new_line);
        return;  // Memory allocation failed
    }
    strcpy(new_line->text, text);

    // Insert in sorted order
    if (program_head == NULL || program_head->line_number > line_number) {
        new_line->next = program_head;
        program_head = new_line;
    } else {
        ProgramLine* current = program_head;
        while (current->next != NULL &&
               current->next->line_number < line_number) {
            current = current->next;
        }
        new_line->next = current->next;
        current->next = new_line;
    }
}
/*--------------------------------------------------------------------------------------------------------------------*/
static Function* find_function(const char* name) {
    Function* current = functions_head;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}
/*--------------------------------------------------------------------------------------------------------------------*/
static int register_function(const char* name,
                             int arg_count,
                             double (*func_ptr)(double[], int)) {
    if (find_function(name) != NULL) {
        return -1;  // Function already exists
    }

    // Allocate new function node
    Function* new_func = (Function*)calloc(1, sizeof(Function));
    if (new_func == NULL) {
        return -1;  // Memory allocation failed
    }

    // Allocate memory for function name
    new_func->name = (char*)calloc(strlen(name) + 1, sizeof(char));
    if (new_func->name == NULL) {
        free(new_func);
        return -1;  // Memory allocation failed
    }

    // Set function properties
    strcpy(new_func->name, name);
    new_func->arg_count = arg_count;
    new_func->func_ptr = func_ptr;
    new_func->next = NULL;

    // Add to front of linked list
    new_func->next = functions_head;
    functions_head = new_func;

    return 0;  // Success
}
/*--------------------------------------------------------------------------------------------------------------------*/
static double func_sin(double args[], int count) {
    if (count != 1)
        return NAN;
    return sin(args[0]);
}
/*--------------------------------------------------------------------------------------------------------------------*/
static double func_cos(double args[], int count) {
    if (count != 1)
        return NAN;
    return cos(args[0]);
}
/*--------------------------------------------------------------------------------------------------------------------*/
static double func_tan(double args[], int count) {
    if (count != 1)
        return NAN;
    return tan(args[0]);
}
/*--------------------------------------------------------------------------------------------------------------------*/
static double func_sqrt(double args[], int count) {
    if (count != 1)
        return NAN;
    if (args[0] < 0)
        return NAN;
    return sqrt(args[0]);
}
/*--------------------------------------------------------------------------------------------------------------------*/
static double func_abs(double args[], int count) {
    if (count != 1)
        return NAN;
    return fabs(args[0]);
}
/*--------------------------------------------------------------------------------------------------------------------*/
static double func_pow(double args[], int count) {
    if (count != 2)
        return NAN;
    return pow(args[0], args[1]);
}
/*--------------------------------------------------------------------------------------------------------------------*/
static double func_log(double args[], int count) {
    if (count != 1)
        return NAN;
    if (args[0] <= 0)
        return NAN;
    return log(args[0]);
}
/*--------------------------------------------------------------------------------------------------------------------*/
static double func_log10(double args[], int count) {
    if (count != 1)
        return NAN;
    if (args[0] <= 0)
        return NAN;
    return log10(args[0]);
}
/*--------------------------------------------------------------------------------------------------------------------*/
static double func_exp(double args[], int count) {
    if (count != 1)
        return NAN;
    return exp(args[0]);
}
/*--------------------------------------------------------------------------------------------------------------------*/
static double func_floor(double args[], int count) {
    if (count != 1)
        return NAN;
    return floor(args[0]);
}
/*--------------------------------------------------------------------------------------------------------------------*/
static double func_ceil(double args[], int count) {
    if (count != 1)
        return NAN;
    return ceil(args[0]);
}
/*--------------------------------------------------------------------------------------------------------------------*/
static double func_min(double args[], int count) {
    if (count < 1)
        return NAN;
    double min_val = args[0];
    for (int i = 1; i < count; i++) {
        if (args[i] < min_val)
            min_val = args[i];
    }
    return min_val;
}
/*--------------------------------------------------------------------------------------------------------------------*/
static double func_max(double args[], int count) {
    if (count < 1)
        return NAN;
    double max_val = args[0];
    for (int i = 1; i < count; i++) {
        if (args[i] > max_val)
            max_val = args[i];
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
/*--------------------------------------------------------------------------------------------------------------------*/
/* Program line management functions (now using linked list) */
static void delete_program_line(int line_number) {
    ProgramLine* to_delete = find_program_line(line_number);
    if (to_delete == NULL) {
        return;  // Line not found
    }

    // Remove from list
    if (to_delete == program_head) {
        program_head = to_delete->next;
    } else {
        ProgramLine* prev = program_head;
        while (prev->next != to_delete) {
            prev = prev->next;
        }
        prev->next = to_delete->next;
    }

    free(to_delete->text);
    free(to_delete);
}
/*--------------------------------------------------------------------------------------------------------------------*/
static void clear_program(void) {
    clear_all_program_lines();
}
/*--------------------------------------------------------------------------------------------------------------------*/
static void list_program(void) {
    ProgramLine* current = program_head;
    while (current != NULL) {
        safe_printf("%d %s\n", current->line_number, current->text);
        current = current->next;
    }
}
/*--------------------------------------------------------------------------------------------------------------------*/
static void insert_program_line(int line_number, const char* text) {
    insert_program_line_sorted(line_number, text);
}
/*--------------------------------------------------------------------------------------------------------------------*/
static int run_program(void) {
    goto_target = -1;      // Reset GOTO target
    for_stack_top = -1;    // Reset FOR stack
    gosub_stack_top = -1;  // Reset GOSUB stack

    ProgramLine* current_line = program_head;

    while (current_line != NULL) {
        // Track current line for FOR loops (we'll need to adapt this)
        current_line_index =
            current_line->line_number;  // Use line number as identifier
        double result;

        // Update FOR stack with current line index
        const char* p = current_line->text;
        while (isspace((unsigned char)*p))
            p++;

        if (strncmp(p, "FOR", 3) == 0 &&
            (isspace((unsigned char)p[3]) || p[3] == '\0')) {
            // About to execute a FOR statement, prepare to update stack
        }

        const char* error = NULL;
        int ret = expr_eval(current_line->text, &result, &error);
        if (ret != 0) {
            safe_printf("Error in line %d: %s\n", current_line->line_number,
                        error ? error : "Unknown error");
            return -1;  // Stop execution on error
        }

        // Check if GOTO was executed (from GOTO, IF-THEN, NEXT, or
        // GOSUB/RETURN)
        if (goto_target != -1) {
            // Check for special end-of-program value
            if (goto_target == -2) {
                // RETURN past end of program - stop execution
                break;
            }

            // Find the target line
            ProgramLine* target_line = find_program_line(goto_target);
            if (target_line == NULL) {
                safe_printf("Error: line %d not found\n", goto_target);
                return -1;
            }

            goto_target = -1;            // Reset GOTO target
            current_line = target_line;  // Jump to target line
            continue;
        }

        current_line = current_line->next;  // Move to next line
    }
    return 0;  // Success
}
/*--------------------------------------------------------------------------------------------------------------------*/
/* Handle special BASIC commands */
static int handle_basic_command(const char* input) {
    const char* p = input;

    // Skip leading whitespace
    while (isspace((unsigned char)*p))
        p++;

    if (strncmp(p, "LIST", 4) == 0 &&
        (isspace((unsigned char)p[4]) || p[4] == '\0')) {
        list_program();
        return 1;  // Command handled
    }

    if (strncmp(p, "RUN", 3) == 0 &&
        (isspace((unsigned char)p[3]) || p[3] == '\0')) {
        run_program();
        return 1;  // Command handled
    }

    if (strncmp(p, "NEW", 3) == 0 &&
        (isspace((unsigned char)p[3]) || p[3] == '\0')) {
        clear_program();
        safe_print("Program cleared\n");
        return 1;  // Command handled
    }

    return 0;  // Not a special command
}
/*--------------------------------------------------------------------------------------------------------------------*/
/* Parse and execute a PRINT statement */
static double parse_print_statement(Parser* p) {
    // Skip the "PRINT" keyword (already matched)
    p->s += 5;  // Skip "PRINT"

    p->s = skip_ws(p->s);

    // Handle empty PRINT (just print newline)
    if (*p->s == '\0') {
        safe_print("\n");
        return 0.0;
    }

    // Parse and print expressions separated by commas
    int first = 1;
    do {
        if (!first) {
            safe_print(" ");  // Space between expressions
        }
        first = 0;

        double value = parse_expr(p);
        if (p->err) {
            printf("!\n");
            return NAN;
        }

        safe_printf("%.*g", 15, value);

        p->s = skip_ws(p->s);
        if (*p->s == ',') {
            p->s++;  // consume comma
            p->s = skip_ws(p->s);
        } else {
            break;
        }
    } while (*p->s != '\0');

    safe_print("\n");  // End with newline
    return 0.0;        // PRINT statements don't return meaningful values
}
/*--------------------------------------------------------------------------------------------------------------------*/
/* Parse and execute a GOTO statement */
static double parse_goto_statement(Parser* p) {
    // Skip the "GOTO" keyword (already matched)
    p->s += 4;  // Skip "GOTO"

    p->s = skip_ws(p->s);

    // Parse the target line number
    if (!isdigit((unsigned char)*p->s)) {
        p->err = "GOTO requires a line number";
        return NAN;
    }

    char* endptr;
    long target_line = strtol(p->s, &endptr, 10);

    if (target_line < 0 || target_line > 65535) {
        p->err = "invalid GOTO line number";
        return NAN;
    }

    p->s = endptr;

    // Set the goto target
    goto_target = (int)target_line;

    return 0.0;  // GOTO statements don't return meaningful values
}
/*--------------------------------------------------------------------------------------------------------------------*/
/* Parse a comparison (expr op expr) and return 1.0 for true, 0.0 for false */
static double parse_comparison(Parser* p) {
    double left = parse_expr(p);
    if (p->err)
        return NAN;

    p->s = skip_ws(p->s);

    // Parse comparison operator
    char op1 = *p->s;
    char op2 = '\0';

    if (op1 == '>' || op1 == '<' || op1 == '=') {
        p->s++;
        // Check for two-character operators
        if (*p->s == '=' || (*p->s == '>' && op1 == '<')) {
            op2 = *p->s++;
        }
    } else {
        p->err = "expected comparison operator";
        return NAN;
    }

    double right = parse_expr(p);
    if (p->err)
        return NAN;

    // Evaluate comparison
    if (op1 == '>' && op2 == '\0') {
        return (left > right) ? 1.0 : 0.0;
    } else if (op1 == '<' && op2 == '\0') {
        return (left < right) ? 1.0 : 0.0;
    } else if (op1 == '>' && op2 == '=') {
        return (left >= right) ? 1.0 : 0.0;
    } else if (op1 == '<' && op2 == '=') {
        return (left <= right) ? 1.0 : 0.0;
    } else if (op1 == '=' && op2 == '\0') {
        return (left == right) ? 1.0 : 0.0;
    } else if (op1 == '<' && op2 == '>') {
        return (left != right) ? 1.0 : 0.0;
    } else {
        p->err = "unknown comparison operator";
        return NAN;
    }
}
/*--------------------------------------------------------------------------------------------------------------------*/
/* Parse and execute an IF statement */
static double parse_if_statement(Parser* p) {
    // Skip the "IF" keyword (already matched)
    p->s += 2;  // Skip "IF"

    p->s = skip_ws(p->s);

    // Parse the condition
    double condition = parse_comparison(p);
    if (p->err)
        return NAN;

    p->s = skip_ws(p->s);

    // Expect THEN keyword
    if (strncmp(p->s, "THEN", 4) != 0) {
        p->err = "expected THEN after IF condition";
        return NAN;
    }
    p->s += 4;  // Skip "THEN"

    p->s = skip_ws(p->s);

    // Check if condition is true (non-zero)
    if (condition != 0.0) {
        // Condition is true, execute the THEN part

        // Check if THEN is followed by a line number
        if (isdigit((unsigned char)*p->s)) {
            // Parse line number and set GOTO target
            char* endptr;
            long target_line = strtol(p->s, &endptr, 10);

            if (target_line < 0 || target_line > 65535) {
                p->err = "invalid IF-THEN line number";
                return NAN;
            }

            p->s = endptr;
            goto_target = (int)target_line;
            return 0.0;
        } else {
            // Parse and execute statement
            return parse_statement(p);
        }
    } else {
        // Condition is false, skip the THEN part
        // Just advance to end of line (we'll ignore the THEN part)
        while (*p->s != '\0')
            p->s++;
        return 0.0;
    }
}
/*--------------------------------------------------------------------------------------------------------------------*/
/* Parse and execute a FOR statement */
static double parse_for_statement(Parser* p) {
    // Skip the "FOR" keyword (already matched)
    p->s += 3;  // Skip "FOR"

    p->s = skip_ws(p->s);

    // Parse variable name
    if (!is_alpha_or_underscore(*p->s)) {
        p->err = "expected variable name after FOR";
        return NAN;
    }

    char var_name[MAX_VAR_NAME];
    int len = 0;
    while (len < MAX_VAR_NAME - 1 && is_alnum_or_underscore(*p->s)) {
        var_name[len++] = *p->s++;
    }
    var_name[len] = '\0';

    p->s = skip_ws(p->s);

    // Expect '='
    if (*p->s != '=') {
        p->err = "expected '=' after FOR variable";
        return NAN;
    }
    p->s++;

    // Parse start value
    double start_val = parse_expr(p);
    if (p->err)
        return NAN;

    p->s = skip_ws(p->s);

    // Expect TO
    if (strncmp(p->s, "TO", 2) != 0) {
        p->err = "expected TO after FOR start value";
        return NAN;
    }
    p->s += 2;

    // Parse end value
    double end_val = parse_expr(p);
    if (p->err)
        return NAN;

    p->s = skip_ws(p->s);

    // Check for optional STEP
    double step_val = 1.0;
    if (strncmp(p->s, "STEP", 4) == 0 &&
        (isspace((unsigned char)p->s[4]) || p->s[4] == '\0')) {
        p->s += 4;
        step_val = parse_expr(p);
        if (p->err)
            return NAN;
    }

    // Check for stack overflow
    if (for_stack_top >= MAX_FOR_LOOPS - 1) {
        p->err = "FOR loop nesting too deep";
        return NAN;
    }

    // Push new FOR loop onto stack
    for_stack_top++;
    ForLoop* loop = &for_stack[for_stack_top];
    strncpy(loop->var_name, var_name, MAX_VAR_NAME - 1);
    loop->var_name[MAX_VAR_NAME - 1] = '\0';
    loop->start_value = start_val;
    loop->end_value = end_val;
    loop->step_value = step_val;
    loop->for_line_number =
        current_line_index;  // Use current executing line number
    loop->next_line_number = -1;

    // Set the loop variable to start value
    set_variable(var_name, start_val);

    return 0.0;
}
/*--------------------------------------------------------------------------------------------------------------------*/
/* Parse and execute a NEXT statement */
static double parse_next_statement(Parser* p) {
    // Skip the "NEXT" keyword (already matched)
    p->s += 4;  // Skip "NEXT"

    p->s = skip_ws(p->s);

    // Parse variable name
    if (!is_alpha_or_underscore(*p->s)) {
        p->err = "expected variable name after NEXT";
        return NAN;
    }

    char var_name[MAX_VAR_NAME];
    int len = 0;
    while (len < MAX_VAR_NAME - 1 && is_alnum_or_underscore(*p->s)) {
        var_name[len++] = *p->s++;
    }
    var_name[len] = '\0';

    // Check if there's a matching FOR loop
    if (for_stack_top < 0) {
        p->err = "NEXT without matching FOR";
        return NAN;
    }

    ForLoop* loop = &for_stack[for_stack_top];
    if (strcmp(loop->var_name, var_name) != 0) {
        p->err = "NEXT variable doesn't match FOR variable";
        return NAN;
    }

    // Get current variable value and increment it
    double current_val = get_variable(var_name);
    if (isnan(current_val)) {
        p->err = "FOR variable not found";
        return NAN;
    }

    current_val += loop->step_value;

    // Check if loop should continue
    int continue_loop;
    if (loop->step_value > 0) {
        continue_loop = (current_val <= loop->end_value);
    } else {
        continue_loop = (current_val >= loop->end_value);
    }

    if (continue_loop) {
        // Update variable and jump back to the line AFTER the FOR statement
        set_variable(var_name, current_val);

        // Find the next line after the FOR statement
        ProgramLine* next_line = find_next_program_line(loop->for_line_number);
        if (next_line != NULL) {
            goto_target = next_line->line_number;
        }
        // If no next line found, continue to end of program
    } else {
        // Loop finished, pop from stack
        for_stack_top--;
    }

    return 0.0;
}
/*--------------------------------------------------------------------------------------------------------------------*/
/* Parse and execute a GOSUB statement */
static double parse_gosub_statement(Parser* p) {
    // Skip the "GOSUB" keyword (already matched)
    p->s += 5;  // Skip "GOSUB"

    p->s = skip_ws(p->s);

    // Parse the target line number
    if (!isdigit((unsigned char)*p->s)) {
        p->err = "GOSUB requires a line number";
        return NAN;
    }

    char* endptr;
    long target_line = strtol(p->s, &endptr, 10);

    if (target_line < 0 || target_line > 65535) {
        p->err = "invalid GOSUB line number";
        return NAN;
    }

    p->s = endptr;

    // Check for stack overflow
    if (gosub_stack_top >= MAX_GOSUB_STACK - 1) {
        p->err = "GOSUB nesting too deep";
        return NAN;
    }

    // Push current line index + 1 (return address) onto GOSUB stack
    gosub_stack_top++;
    // Store the line number of the next line
    ProgramLine* next_line = find_next_program_line(current_line_index);
    if (next_line != NULL) {
        gosub_stack[gosub_stack_top] = next_line->line_number;
    } else {
        // No next line - mark as end of program
        gosub_stack[gosub_stack_top] = -2;
    }

    // Set the goto target
    goto_target = (int)target_line;

    return 0.0;
}
/*--------------------------------------------------------------------------------------------------------------------*/
/* Parse and execute a RETURN statement */
static double parse_return_statement(Parser* p) {
    // Skip the "RETURN" keyword (already matched)
    p->s += 6;  // Skip "RETURN"

    // Check if there's a matching GOSUB
    if (gosub_stack_top < 0) {
        p->err = "RETURN without matching GOSUB";
        return NAN;
    }

    // Pop return address from stack
    int return_line_number = gosub_stack[gosub_stack_top];
    gosub_stack_top--;

    // Jump back to the return address
    if (return_line_number == -2) {
        // Return past end of program - stop execution
        goto_target = -2;  // Special value to indicate end of program
    } else {
        goto_target = return_line_number;
    }

    return 0.0;
}
/*--------------------------------------------------------------------------------------------------------------------*/
/* Parse and execute an END statement */
static double parse_end_statement(Parser* p) {
    // Skip the "END" keyword (already matched)
    p->s += 3;  // Skip "END"

    // Set goto target to special end-of-program value
    goto_target = -2;  // Special value to indicate end of program

    return 0.0;
}
/*--------------------------------------------------------------------------------------------------------------------*/
static const char* skip_ws(const char* p) {
    while (*p && isspace((unsigned char)*p)) {
        p++;
    }
    return p;
}
/*--------------------------------------------------------------------------------------------------------------------*/
static int accept(Parser* p, char c) {
    p->s = skip_ws(p->s);
    if (*p->s == c) {
        p->s++;
        return 1;
    }
    return 0;
}
/*--------------------------------------------------------------------------------------------------------------------*/
static void expect(Parser* p, char c) {
    p->s = skip_ws(p->s);
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
    p->s = skip_ws(p->s);
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
    p->s = skip_ws(p->s);

    if (!is_alpha_or_underscore(*p->s)) {
        p->err = "expected variable name";
        return NAN;
    }

    char var_name[MAX_VAR_NAME];
    int len = 0;

    // Parse variable name (starts with letter or underscore, followed by
    // alphanumeric or underscore)
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
    if (p->err)
        return NAN;

    double args[MAX_FUNC_ARGS];
    int arg_count = 0;

    // Parse arguments
    p->s = skip_ws(p->s);
    if (*p->s != ')') {  // Function has arguments
        do {
            if (arg_count >= MAX_FUNC_ARGS) {
                p->err = "too many function arguments";
                return NAN;
            }

            args[arg_count] = parse_expr(p);
            if (p->err)
                return NAN;
            arg_count++;

            p->s = skip_ws(p->s);
            if (*p->s == ',') {
                p->s++;  // consume comma
            } else {
                break;
            }
        } while (1);
    }

    expect(p, ')');
    if (p->err)
        return NAN;

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
    p->s = skip_ws(p->s);
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

        p->s = skip_ws(p->s);

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
        p->s = skip_ws(p->s);
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
        p->s = skip_ws(p->s);
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
    p->s = skip_ws(p->s);

    // Check for PRINT statement
    if (strncmp(p->s, "PRINT", 5) == 0 &&
        (isspace((unsigned char)p->s[5]) || p->s[5] == '\0')) {
        return parse_print_statement(p);
    }

    // Check for GOTO statement
    if (strncmp(p->s, "GOTO", 4) == 0 &&
        (isspace((unsigned char)p->s[4]) || p->s[4] == '\0')) {
        return parse_goto_statement(p);
    }

    // Check for IF statement
    if (strncmp(p->s, "IF", 2) == 0 &&
        (isspace((unsigned char)p->s[2]) || p->s[2] == '\0')) {
        return parse_if_statement(p);
    }

    // Check for FOR statement
    if (strncmp(p->s, "FOR", 3) == 0 &&
        (isspace((unsigned char)p->s[3]) || p->s[3] == '\0')) {
        return parse_for_statement(p);
    }

    // Check for NEXT statement
    if (strncmp(p->s, "NEXT", 4) == 0 &&
        (isspace((unsigned char)p->s[4]) || p->s[4] == '\0')) {
        return parse_next_statement(p);
    }

    // Check for GOSUB statement
    if (strncmp(p->s, "GOSUB", 5) == 0 &&
        (isspace((unsigned char)p->s[5]) || p->s[5] == '\0')) {
        return parse_gosub_statement(p);
    }

    // Check for RETURN statement
    if (strncmp(p->s, "RETURN", 6) == 0 &&
        (isspace((unsigned char)p->s[6]) || p->s[6] == '\0')) {
        return parse_return_statement(p);
    }

    // Check for END statement
    if (strncmp(p->s, "END", 3) == 0 &&
        (isspace((unsigned char)p->s[3]) || p->s[3] == '\0')) {
        return parse_end_statement(p);
    }

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

        p->s = skip_ws(p->s);

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
void expr_init(void (*print_func)(const char* str)) {
    print_function = print_func;

    clear_all_variables();
    clear_all_functions();
    clear_all_program_lines();

    goto_target = -1;
    for_stack_top = -1;
    current_line_index = -1;
    gosub_stack_top = -1;
    init_builtin_functions();
}
/*--------------------------------------------------------------------------------------------------------------------*/
int expr_eval(const char* expr, double* result, const char** error) {
    Parser p = {.start = expr, .s = expr, .err = NULL};
    double v = parse_statement(&p);
    if (p.err) {
        if (error) {
            *error = p.err;
        }
        return -1;  // Error: parsing failed
    }
    p.s = skip_ws(p.s);
    if (*p.s != '\0') {
        *error = "Unexpected characters at end";
        return -1;  // Error: unexpected characters at end
    }
    *result = v;
    return 0;
}
/*--------------------------------------------------------------------------------------------------------------------*/
int expr_register_function(const char* name,
                           int arg_count,
                           double (*func_ptr)(double[], int)) {
    return register_function(name, arg_count, func_ptr);
}
/*--------------------------------------------------------------------------------------------------------------------*/
/* Parse a line that may start with a line number */
int expr_parse_line(const char* input, double* result, const char** error) {
    const char* p = input;

    while (isspace((unsigned char)*p))
        p++;

    // Check for special BASIC commands first
    if (handle_basic_command(input)) {
        *result = 0;
        return 3;  // Special command executed
    }

    // Check if line starts with a number (line number)
    if (isdigit((unsigned char)*p)) {
        // Parse line number
        char* endptr;
        long line_num = strtol(p, &endptr, 10);

        if (line_num < 0 || line_num > 65535) {
            return -1;  // Invalid line number
        }

        p = endptr;

        // Skip whitespace after line number
        while (isspace((unsigned char)*p))
            p++;

        if (*p == '\0') {
            // Just a line number - delete the line
            delete_program_line((int)line_num);
            *result = line_num;
            return 1;  // Line deleted
        } else {
            // Line number followed by statement - store the line
            insert_program_line((int)line_num, p);
            *result = line_num;
            return 2;  // Line stored
        }
    } else {
        // No line number - immediate mode evaluation
        return expr_eval(input, result, error);
    }
}
/*--------------------------------------------------------------------------------------------------------------------*/
