# Architecture

G2Basic is a line-oriented tree-less interpreter: it parses and executes each
statement directly from its source text, every time the statement runs. There
is no tokenizer, bytecode, or syntax tree.

## Source layout

| Path | Responsibility |
| --- | --- |
| `src/g2basic.h` | Public API |
| `src/g2basic.c` | Line handling, program store, parser and evaluator, statements, runtime state |
| `src/g2basic_math.h` | Internal hook that registers the built-in functions |
| `src/g2basic_math.c` | Built-ins using libm |
| `src/g2basic_math_crude.c` | Built-ins using approximations, without libm |
| `src/g2basic_math_none.c` | Only `min` and `max` |
| `examples/interactive/` | Standard-input REPL used for manual and CI testing |
| `tests/check_math.py` | Accuracy and linkage checks for both math backends |
| `docs/` | Guides and the Doxygen main page, theme, and header |

CMake compiles `g2basic.c` and exactly one backend into the `g2basic` library.
The backend is chosen at configure time, not with preprocessor definitions.

## Processing a line

`g2basic_parse()` classifies each input line:

1. **Commands.** `LIST`, `RUN`, and `NEW` are matched case-insensitively as the
   first word and handled directly.
2. **Program lines.** A leading number is parsed with `strtol`. The rest of the
   line is copied into the program store, or the numbered line is deleted.
3. **Immediate statements.** Anything else goes to the statement parser.

The statement parser is recursive descent over the source text:

```text
statement  := keyword-statement | assignment | expr
assignment := identifier '=' expr
expr       := term (('+' | '-') term)*
term       := factor (('*' | '/') factor)*
factor     := ('+' | '-') factor | '(' expr ')' | identifier '(' args ')'
            | identifier | number
```

Keyword statements are dispatched from a table that pairs each keyword with
its parser function. Each parser evaluates as it parses: `PRINT` prints each
value as it is read, and `FOR` pushes its loop state immediately. Errors are
recorded as a message pointer in the parser state and unwind the recursion.

## Runtime state

All state is file-scope in `g2basic.c`, which is why there is one interpreter
per program.

| State | Structure | Notes |
| --- | --- | --- |
| Variables | Singly linked list of name and `double` | New names are inserted at the head. NaN marks "undefined". |
| Functions | Singly linked list of name, argument count, and C pointer | Duplicate names are rejected. |
| Program | Singly linked list sorted by line number | Each line holds a heap copy of its text. |
| FOR stack | Linked list of variable, bounds, step, and `FOR` line | Pushed by `FOR`, popped when `NEXT` finishes the loop. |
| GOSUB stack | Linked list of return line numbers | -2 means "end of program". |
| Jump target | Integer set by `GOTO`, `GOSUB`, `RETURN`, `IF ... THEN n`, `NEXT`, and `END` | -1 means none; -2 means stop. |

Lookups are linear searches, so a program with many lines or variables slows
jumps and variable access proportionally.

## Running a program

`RUN` clears the FOR and GOSUB stacks, then walks the sorted program list. For
each line it re-parses the text and executes it. After each line, the runner
checks the jump target: -1 moves to the next line, -2 stops, and any other value
jumps to that line or reports `Error: line N not found`.

Loops and subroutines are expressed with that jump target. `NEXT` jumps to the
line after the recorded `FOR` line while the loop continues. `GOSUB` records the
line after the current one, and `RETURN` jumps there. Because jumps name line
numbers, `FOR` and `GOSUB` must be the only statement on their line, which the
single-statement line format already ensures.

## Output

Output goes through the host's text callback. Formatted messages use a 512-byte
stack buffer and `vsnprintf`. PRINT formats numbers with `%.15g` unless the host
installed a number formatter.

## Memory

Every list node and string is allocated with `calloc` and freed when its line,
loop, or subroutine frame ends, or by `g2basic_init()`. There are no fixed
limits apart from 8 function arguments, the 0 to 65535 line-number range, and
64-byte buffers for formatted error messages. The parser's recursion depth is
bounded by `G2BASIC_MAX_NESTING`: `enter_nesting()` counts each parenthesis,
unary sign, function-call argument list, and `THEN` statement, and fails with
`expression too deeply nested` past the limit, so C stack use per line has a
fixed upper bound.

## Known issues

These are current behaviors, documented so hosts can plan for them:

- A failing `PRINT` writes `!` to `stdout` with `printf`, bypassing the output
  callback.
- `g2basic_parse()` writes through `result` and `error` without checking for
  `NULL`, and leaves `error` unset for out-of-range line numbers.
- `RUN` reports failure only through output; its return value is always 3.
- Allocation failures when storing a line or assigning a variable are ignored.
- The no-math backend still registers `min` and `max`.
- `g2basic.c` has an unused parameter and an unused variable, so it does not
  compile with `-Wall -Wextra -Werror`. CMake's
  `add_compile_options(... -Werror)` call comes after the library target is
  created, so it applies only to the example, not to the library.
