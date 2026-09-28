# C coding standard

These rules apply to new and changed code in `src/`, `examples/`, and `tests/`.
They describe conventions the code already follows, plus rules needed because
G2Basic runs inside embedded hosts such as HomeCore. Existing deviations are
listed under [known issues](architecture.md#known-issues); fix them when you
change the affected code, not in unrelated changes.

## Language and portability

- Write C99 that also compiles as C11. Do not rely on compiler extensions or
  platform headers; the library uses only the standard C library.
- The library must build for bare-metal targets with newlib-nano and software
  floating point. Do not add dependencies on files, threads, time, or
  environment variables.
- Only `g2basic_math.c` may call libm functions. The other backends may use
  `<math.h>` only for constants and classification macros such as `NAN` and
  `isnan`. `tests/check_math.py` verifies this for the approximation backend.
- Cast `char` values to `unsigned char` before passing them to `<ctype.h>`
  functions.
- Avoid variable-length arrays and large stack buffers. Every recursive parser
  path must pass through `enter_nesting()` so `G2BASIC_MAX_NESTING` bounds its
  depth.

## Formatting and naming

- Follow `.clang-format`: Chromium style, four-space indentation, 80 columns,
  left-aligned pointers (`char* name`). Keep formatting changes to edited code.
- Separate top-level definitions with the existing
  `/*----...----*/` separator lines.
- Prefix public functions and macros with `g2basic_` or `G2BASIC_`. Mark every
  other function and file-scope object `static`.
- Use `snake_case` for functions and variables. Internal structure types use
  `CamelCase` names, as in `ProgramLine` and `ForLoop`.

## Interfaces

- Declare public functions only in `g2basic.h`, and internal cross-file hooks in
  internal headers such as `g2basic_math.h`. Keep headers self-contained, with
  include guards.
- Document every header declaration with Doxygen, inside its group. State
  pointer ownership and lifetime, whether `NULL` is allowed, and every return
  value. See [API documentation](development.md#api-documentation).
- Keep the public API backward compatible where possible. Every change to
  shipped code needs a version increment and changelog entry; see
  [versioning](versioning.md).

## Memory and errors

- Check every allocation. On failure, free what the operation already
  allocated, leave existing state unchanged, and report an error. Do not
  ignore the failure.
- Every allocation belongs to exactly one list or owner and is freed when that
  entry is removed, and by `g2basic_init()`.
- Report parse and runtime errors by setting the parser's error message to a
  string literal or a static buffer, then return. Messages are lowercase
  without a trailing period, and name the offending item in single quotes:
  `unknown function 'name'`.
- Built-in functions report invalid input by returning NaN.
- Validate public API arguments that can be `NULL`, or document that they must
  not be.

## Output

- Produce all output through the host's callbacks (the text callback and the
  optional number formatter). Library code must not call `printf`, `puts`, or
  write to `stdout` directly.
- Keep formatted output within the 512-byte buffer used by the output helper.

## Warnings and checks

- Code must compile without warnings with `-Wall -Wextra -Wpedantic`, both as
  part of the library and in tests.
- Run `python3 tests/check_math.py` after changing a math backend, and the
  smoke test in [development](development.md#tests) after interpreter changes.
- Run `doxygen Doxyfile` after changing a header; it must finish without
  warnings.
