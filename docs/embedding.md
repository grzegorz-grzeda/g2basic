# Embedding

G2Basic is a C library with one public header, `g2basic.h`. This guide covers
adding it to a build and using the API safely. The
[API reference](https://grzegorz-grzeda.github.io/g2basic/) documents each
function; the language is in the [language reference](language.md).

## Adding the library

Add the repository to your project, for example as a submodule under
`external/g2basic`, then link the `g2basic` target:

```cmake
add_subdirectory(external/g2basic)
target_link_libraries(your_app PRIVATE g2basic)
```

The target exports `src/` as its include directory, so code includes
`"g2basic.h"`. When added this way, examples and the built-in math functions
are off, leaving only `min` and `max`. To get the math functions without libm,
set both options before `add_subdirectory()`:

```cmake
set(G2BASIC_ENABLE_MATH_FUNCTIONS ON CACHE BOOL "" FORCE)
set(G2BASIC_ENABLE_MATH OFF CACHE BOOL "" FORCE)
```

See [build options](development.md#build-options) for all combinations. The
optional `docs` target is defined only in standalone builds, so it does not
clash with a host project's targets.

Without CMake, compile `src/g2basic.c` and exactly one backend:
`src/g2basic_math.c` (link libm), `src/g2basic_math_crude.c`, or
`src/g2basic_math_none.c`. No preprocessor definitions are needed.

## Lifecycle

```c
#include <stdio.h>
#include "g2basic.h"

static void print_text(const char* text) {
    fputs(text, stdout);
}

void run_line(const char* line) {
    double result;
    const char* error = NULL;

    if (g2basic_parse(line, &result, &error) < 0) {
        printf("Error: %s\n", error ? error : "invalid line number");
    }
}

int main(void) {
    g2basic_init(print_text);
    run_line("10 PRINT 6 * 7");
    run_line("RUN");  // prints 42
    return 0;
}
```

1. Call `g2basic_init()` before anything else. Calling it again frees the
   program, variables, and all registered functions, then registers the
   built-ins again.
2. Register custom functions after each `g2basic_init()`.
3. Pass one line at a time to `g2basic_parse()`.

There is no shutdown function. Memory is released only by the next
`g2basic_init()` call.

## The parse contract

| Return | Meaning | `result` |
| --- | --- | --- |
| 0 | Immediate statement succeeded | Statement value (0 for statements without one) |
| 1 | Line deleted (or did not exist) | Line number |
| 2 | Line stored | Line number |
| 3 | `LIST`, `RUN`, or `NEW` ran | 0 |
| -1 | Error | Unchanged |

- `result` and `error` must not be `NULL`; several paths write through them
  unconditionally.
- `error` is written only on failure. Set it to `NULL` before each call, as
  above. It is not set when a line number is outside 0 to 65535.
- Error strings are static. Some are rebuilt by the next call, so copy a message
  if you need it later. Never free them.
- `RUN` reports runtime errors through the output callback, not through the
  return value, which is 3 even when the program failed.
- Pass lines without the line terminator. A trailing newline is stored as part
  of a program line and printed again by `LIST`.

## Output

All output goes through the callback passed to `g2basic_init()`: PRINT values,
separators and newlines, `LIST`, and `RUN` errors. Strings are valid only during
the callback. Messages longer than 511 bytes are truncated. `NULL` disables
output.

`g2basic_set_number_output()` replaces number formatting, for example on a
target whose `printf` lacks floating-point support:

```c
static void print_number(double value) {
    char text[24];
    snprintf(text, sizeof(text), "%ld", (long)value);  // integers only
    print_text(text);
}

g2basic_init(print_text);
g2basic_set_number_output(print_number);
```

The formatter receives only PRINT values; separators and newlines still go to
the text callback. `g2basic_init()` resets it.

One exception: a `PRINT` whose expression fails writes `!` and a newline
directly to `stdout` with `printf`, bypassing the callback.

## Custom functions

```c
static double clamp(double args[], int count) {
    (void)count;  // The interpreter guarantees 3.
    double value = args[0];
    return value < args[1] ? args[1] : (value > args[2] ? args[2] : value);
}

g2basic_register_function("clamp", 3, clamp);  // BASIC: PRINT clamp(x, 0, 10)
```

- `arg_count` is the exact number of arguments (0 to 8), or -1 for any number
  up to 8. Calls with the wrong count fail before your function runs.
- Names are copied and case-sensitive. They must be valid identifiers to be
  callable; this is not validated.
- Registration returns -1 if the name exists. Built-ins cannot be replaced.
- There is no error channel. A NaN result is an ordinary value, and a variable
  assigned NaN then reads as undefined.

Functions are the way to reach hardware from BASIC. HomeCore, for example,
registers `millis()` to read its uptime clock.

## Constraints for embedded hosts

- **One interpreter.** State is global, so the API is not reentrant or
  thread-safe. Do not call it from interrupt handlers.
- **Heap use.** Variables, functions, program lines, and loop and subroutine
  frames are allocated with `calloc`. Nothing limits their number except
  memory. An allocation failure while storing a line or assigning a variable is
  silently ignored.
- **Stack use.** Expressions are parsed recursively. `G2BASIC_MAX_NESTING`
  (default 8 when embedded, 32 standalone) bounds the nesting of parentheses,
  unary signs, function calls, and `IF ... THEN` statements per line; deeper
  lines fail with `expression too deeply nested`. Measured on Cortex-M3 with
  GCC, BASIC needs about 910 bytes of stack at depth 0 (including a 512-byte
  output buffer) plus up to about 230 bytes per level, for nested function
  calls in a Debug build. Size the stack for that and set the option to match:

  ```cmake
  set(G2BASIC_MAX_NESTING 4 CACHE STRING "" FORCE)
  add_subdirectory(external/g2basic)
  ```
- **Blocking.** `RUN` does not return until the program ends. A program that
  loops forever, such as `10 GOTO 10`, never returns. There is no break hook.
- **Floating point.** All values are `double`. On targets with software
  floating point, arithmetic is correspondingly slow.
