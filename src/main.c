/*
 * MIT License
 *
 * Copyright (c) 2024 Grzegorz
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
//======================================================================================================================
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//======================================================================================================================
typedef enum g2basic_character_type {
    G2BASIC_CHARACTER_TYPE_WHITESPACE,
    G2BASIC_CHARACTER_TYPE_DIGIT,
    G2BASIC_CHARACTER_TYPE_LETTER,
    G2BASIC_CHARACTER_TYPE_OPERATOR,
    G2BASIC_CHARACTER_TYPE_PUNCTUATION,
    G2BASIC_CHARACTER_TYPE_STRING_DELIMITER,
    G2BASIC_CHARACTER_TYPE_LINE_TERMINATOR,
    G2BASIC_CHARACTER_TYPE_UNKNOWN
} g2basic_character_type_t;
//======================================================================================================================
typedef enum g2basic_token_type {
    G2BASIC_TOKEN_TYPE_NAME,
    G2BASIC_TOKEN_TYPE_NUMBER,
    G2BASIC_TOKEN_TYPE_STRING,
    G2BASIC_TOKEN_TYPE_OPERATOR,
    G2BASIC_TOKEN_TYPE_LINE_TERMINATOR,
    G2BASIC_TOKEN_TYPE_UNKNOWN
} g2basic_token_type_t;
//======================================================================================================================
typedef struct g2basic_token {
    g2basic_token_type_t type;
    char* value;
} g2basic_token_t;
//======================================================================================================================
typedef struct g2basic_line {
    int line_number;
    g2basic_token_t* tokens;  // Array of tokens in this line
    size_t token_count;       // Number of tokens in this line
} g2basic_line_t;
//======================================================================================================================
static g2basic_token_t tokens[1000];
static size_t token_count = 0;
static g2basic_line_t lines[1000];
static size_t line_count = 0;
//======================================================================================================================
static g2basic_character_type_t g2basic_get_character_type(char input) {
    if (strchr("\n\r", input)) {
        return G2BASIC_CHARACTER_TYPE_LINE_TERMINATOR;
    } else if (isspace((unsigned char)input)) {
        return G2BASIC_CHARACTER_TYPE_WHITESPACE;
    } else if (isdigit((unsigned char)input)) {
        return G2BASIC_CHARACTER_TYPE_DIGIT;
    } else if (isalpha((unsigned char)input) || input == '_') {
        return G2BASIC_CHARACTER_TYPE_LETTER;
    } else if (strchr("+-*/=<>!&|", input)) {
        return G2BASIC_CHARACTER_TYPE_OPERATOR;
    } else if (strchr("()[]{};:,.'", input)) {
        return G2BASIC_CHARACTER_TYPE_PUNCTUATION;
    } else if (input == '"') {
        return G2BASIC_CHARACTER_TYPE_STRING_DELIMITER;
    } else {
        return G2BASIC_CHARACTER_TYPE_UNKNOWN;
    }
}
//======================================================================================================================
static const char* g2basic_get_token_type_name(g2basic_token_type_t type) {
    switch (type) {
        case G2BASIC_TOKEN_TYPE_NAME:
            return "Name";
        case G2BASIC_TOKEN_TYPE_NUMBER:
            return "Number";
        case G2BASIC_TOKEN_TYPE_STRING:
            return "String";
        case G2BASIC_TOKEN_TYPE_OPERATOR:
            return "Operator";
        case G2BASIC_TOKEN_TYPE_LINE_TERMINATOR:
            return "Line Terminator";
        case G2BASIC_TOKEN_TYPE_UNKNOWN:
            return "Unknown";
        default:
            return "Invalid Type";
    }
}
//======================================================================================================================
static void g2basic_parse_input(const char* input) {
    token_count = 0;
    g2basic_token_t* current_token = &tokens[token_count];
    g2basic_token_type_t current_type = G2BASIC_TOKEN_TYPE_UNKNOWN;
    char buffer[256] = {0};
    size_t buffer_length = 0;

    for (; *input; input++) {
        g2basic_character_type_t character_type = g2basic_get_character_type(*input);

        if (current_type == G2BASIC_TOKEN_TYPE_UNKNOWN) {
            if (character_type == G2BASIC_CHARACTER_TYPE_WHITESPACE) {
                continue;
            } else if (character_type == G2BASIC_CHARACTER_TYPE_LINE_TERMINATOR) {
                current_type = G2BASIC_TOKEN_TYPE_LINE_TERMINATOR;
                buffer[buffer_length++] = *input;
                continue;
            } else if (character_type == G2BASIC_CHARACTER_TYPE_DIGIT) {
                current_type = G2BASIC_TOKEN_TYPE_NUMBER;
                buffer[buffer_length++] = *input;
                continue;
            } else if (character_type == G2BASIC_CHARACTER_TYPE_STRING_DELIMITER) {
                current_type = G2BASIC_TOKEN_TYPE_STRING;
                continue;
            } else if (character_type == G2BASIC_CHARACTER_TYPE_LETTER) {
                current_type = G2BASIC_TOKEN_TYPE_NAME;
                buffer[buffer_length++] = *input;
                continue;
            } else if (character_type == G2BASIC_CHARACTER_TYPE_OPERATOR) {
                current_type = G2BASIC_TOKEN_TYPE_OPERATOR;
                buffer[buffer_length++] = *input;
                continue;
            } else {
                printf("Unknown character type for: '%c'\n", *input);
            }
        } else if (current_type == G2BASIC_TOKEN_TYPE_LINE_TERMINATOR) {
            if (character_type == G2BASIC_CHARACTER_TYPE_LINE_TERMINATOR) {
                buffer[buffer_length++] = *input;
            } else {
                buffer[buffer_length] = '\0';
                current_token->type = current_type;
                current_token->value = (char*)calloc(buffer_length + 1, sizeof(char));
                if (current_token->value) {
                    strcpy(current_token->value, buffer);
                }
                token_count++;
                current_token++;
                current_type = G2BASIC_TOKEN_TYPE_UNKNOWN;
                buffer_length = 0;
                input--;
            }
        } else if (current_type == G2BASIC_TOKEN_TYPE_NUMBER) {
            if (character_type == G2BASIC_CHARACTER_TYPE_DIGIT) {
                buffer[buffer_length++] = *input;
                continue;
            } else {
                buffer[buffer_length] = '\0';
                current_token->type = current_type;
                current_token->value = (char*)calloc(buffer_length + 1, sizeof(char));
                if (current_token->value) {
                    strcpy(current_token->value, buffer);
                }
                token_count++;
                current_token++;
                current_type = G2BASIC_TOKEN_TYPE_UNKNOWN;
                buffer_length = 0;
                input--;  // Reprocess the current character
            }
        } else if (current_type == G2BASIC_TOKEN_TYPE_STRING) {
            if (character_type == G2BASIC_CHARACTER_TYPE_STRING_DELIMITER) {
                buffer[buffer_length] = '\0';
                current_token->type = current_type;
                current_token->value = (char*)calloc(buffer_length + 1, sizeof(char));
                if (current_token->value) {
                    strcpy(current_token->value, buffer);
                }
                token_count++;
                current_token++;
                current_type = G2BASIC_TOKEN_TYPE_UNKNOWN;
                buffer_length = 0;
                // Skip the closing string delimiter so no processing is done for it
            } else {
                buffer[buffer_length++] = *input;
            }
        } else if (current_type == G2BASIC_TOKEN_TYPE_NAME) {
            if (character_type == G2BASIC_CHARACTER_TYPE_LETTER || character_type == G2BASIC_CHARACTER_TYPE_DIGIT ||
                *input == '_') {
                buffer[buffer_length++] = *input;
                continue;
            } else {
                buffer[buffer_length] = '\0';
                current_token->type = current_type;
                current_token->value = (char*)calloc(buffer_length + 1, sizeof(char));
                if (current_token->value) {
                    strcpy(current_token->value, buffer);
                }
                token_count++;
                current_token++;
                current_type = G2BASIC_TOKEN_TYPE_UNKNOWN;
                buffer_length = 0;
                input--;  // Reprocess the current character
            }
        } else if (current_type == G2BASIC_TOKEN_TYPE_OPERATOR) {
            if (character_type == G2BASIC_CHARACTER_TYPE_OPERATOR) {
                buffer[buffer_length++] = *input;
                continue;
            } else {
                buffer[buffer_length] = '\0';
                current_token->type = current_type;
                current_token->value = (char*)calloc(buffer_length + 1, sizeof(char));
                if (current_token->value) {
                    strcpy(current_token->value, buffer);
                }
                token_count++;
                current_token++;
                current_type = G2BASIC_TOKEN_TYPE_UNKNOWN;
                buffer_length = 0;
                input--;  // Reprocess the current character
            }
        }
    }
    if (current_type != G2BASIC_TOKEN_TYPE_UNKNOWN) {
        buffer[buffer_length] = '\0';
        current_token->type = current_type;
        current_token->value = (char*)calloc(buffer_length + 1, sizeof(char));
        if (current_token->value) {
            strcpy(current_token->value, buffer);
        }
        token_count++;
    }

    int processing_line = 0;
    int line_started = 0;
    for (size_t i = 0; i < token_count; i++) {
        if (processing_line) {
            if (tokens[i].type == G2BASIC_TOKEN_TYPE_LINE_TERMINATOR) {
                processing_line = 0;
                line_started = 0;
                line_count++;
            } else {
                if (line_started) {
                    line_started = 0;
                    lines[line_count].tokens = tokens + i;
                    lines[line_count].token_count = 1;
                } else {
                    lines[line_count].token_count++;
                }
            }
        } else {
            if (tokens[i].type == G2BASIC_TOKEN_TYPE_LINE_TERMINATOR) {
                continue;
            }
            if (tokens[i].type == G2BASIC_TOKEN_TYPE_NUMBER) {
                printf("Processing line number: %s\n", tokens[i].value);
                processing_line = 1;
                line_started = 1;
                lines[line_count].line_number = atoi(tokens[i].value);
                lines[line_count].token_count = 0;
            } else {
                processing_line = 1;
                line_started = 1;
                lines[line_count].line_number = 0;  // Default line number if not specified
                lines[line_count].token_count = 0;
            }
        }
    }
}
//======================================================================================================================
//======================================================================================================================
int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    const char* input =
        "10 PRINT \"Hello, World 1 time!\"\n"
        "20 LET A = 5\n"
        "30 PRINT A\n"
        "40 LET B = \"\"\n"
        "50 IF A > 0 THEN PRINT \"A is positive\"\n";

    printf("Welcome to G2Basic!\n");
    printf("Input:\n------------------\n%s", input);

    printf("------------------\n");
    g2basic_parse_input(input);

    printf("Parsing completed. Found %zu lines.\n", line_count);
    for (size_t i = 0; i < line_count; i++) {
        printf("Line %d\n", lines[i].line_number);
        for (size_t j = 0; j < lines[i].token_count; j++) {
            printf("  [%zu]: %8.8s '%s'\n", j, g2basic_get_token_type_name(lines[i].tokens[j].type),
                   lines[i].tokens[j].value ? (*lines[i].tokens[j].value == '\n' ? "\\n" : lines[i].tokens[j].value)
                                            : "NULL");
        }
    }

    return 0;
}
//======================================================================================================================
