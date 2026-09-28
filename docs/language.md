# Language reference

G2Basic runs a small, numbers-only BASIC. This page describes what the current
implementation accepts. Host integration is covered in [embedding](embedding.md).

## Lines

Each input line is handled in one of three ways:

| Input | Effect |
| --- | --- |
| `LIST`, `RUN`, or `NEW` | Runs the command. Text after the command word is ignored. |
| Line number, then a statement | Stores the statement, replacing any line with that number. |
| Line number alone | Deletes that line. |
| Anything else | Runs the statement immediately. |

Line numbers range from 0 to 65535. Stored text is kept as typed and is not
checked until it runs. There is one statement per line; `:` separators are not
supported.

An immediate line starting with a digit is read as a line number, so `2 + 3`
stores line 2 as `+ 3`. Use `PRINT 2 + 3` or `(2 + 3)` for an immediate
calculation.

## Values and names

- **Numbers only.** Every value is a C `double`. There are no strings, string
  literals, or arrays; `PRINT "Hello"` fails with `expected number`.
- **Numeric literals** follow C `strtod`: `12`, `3.5`, `.5`, `1e3`, `1e-2`.
  Hexadecimal such as `0x1F` is also accepted.
- **Identifiers** start with a letter or underscore, followed by letters,
  digits, or underscores. Variable and function names are case-sensitive: `x`
  and `X` are different variables, and the built-in functions are lowercase.
- **Keywords** (`PRINT`, `IF`, `THEN`, `FOR`, `TO`, `STEP`, `NEXT`, `GOTO`,
  `GOSUB`, `RETURN`, `END`, and the commands) are case-insensitive. A keyword
  must be followed by a space or the end of the line. `PRINT(1)` is read as a
  call to a function named `PRINT`.
- **Variables** are created by assignment. Reading one that was never assigned
  is the error `undefined variable 'name'`. A variable holding NaN also reads
  as undefined.

## Expressions

| Precedence | Operators |
| --- | --- |
| Highest | `( )`, function calls, unary `+` and `-` |
| | `*`, `/` |
| Lowest | `+`, `-` |

Binary operators are left-associative. There is no power operator (use
`pow(a, b)`), no `MOD`, and no logical operators. Division by zero, including
`0 / 0`, is the error `division by zero`.

Parentheses, unary signs, function calls, and `IF ... THEN` statements can
nest up to `G2BASIC_MAX_NESTING` levels in one line: 32 in a standalone build,
8 by default when embedded, and whatever the host configures (4 on HomeCore's
8 KB board). Sibling groups do not add up: `((1)) + ((2))` nests two levels.

Function calls take up to 8 arguments: `name(arg, ...)`. The interpreter
checks the argument count. See [built-in functions](#built-in-functions).

## Statements

| Statement | Behavior |
| --- | --- |
| `name = expr` | Assigns a variable (there is no `LET`). |
| `expr` | Evaluates an expression. |
| `PRINT expr, ...` | Prints the values separated by single spaces, then a newline. `PRINT` alone prints a newline. |
| `GOTO n` | Continues at line `n`. |
| `GOSUB n` | Saves the next line as the return point and continues at line `n`. |
| `RETURN` | Continues at the most recent `GOSUB` return point. |
| `IF a op b THEN n` | If the comparison is true, continues at line `n`. |
| `IF a op b THEN statement` | If the comparison is true, runs one statement. It can be another `IF`. |
| `FOR v = a TO b [STEP s]` | Starts a loop; see below. |
| `NEXT v` | Ends the innermost loop's body; `v` must name its variable. |
| `END` | Stops the program. |

`IF` compares two expressions with `=`, `<>`, `<`, `>`, `<=`, or `>=`.
Comparisons exist only in `IF`; they are not values. When the comparison is
false, the rest of the line is skipped.

Numbers print with up to 15 significant digits (`0.333333333333333`, `1e+16`,
`inf`). NaN prints as `nan` or `-nan`, depending on the C library. A host can
replace the formatter; see [embedding](embedding.md#output).

### Loops

`FOR` sets the variable to the start value. The body is every line after the
`FOR` line up to `NEXT`, so `FOR` must be on its own line. `NEXT` adds the step
(default 1). With a positive step the loop repeats while the variable is at
most the end value. With a zero or negative step it repeats while the variable
is at least the end value. The body always runs at least once.

```basic
10 FOR I = 10 TO 1 STEP -3
20 PRINT I
30 NEXT I
```

This prints 10, 7, 4, and 1. After a loop finishes, the variable keeps its last
value in range. `STEP 0` makes a loop end after one pass when the start is
below the end, and repeat forever otherwise.

### Subroutines

```basic
10 GOSUB 100
20 PRINT 2
30 END
100 PRINT 1
110 RETURN
```

This prints 1, then 2. If `GOSUB` is on the last line, `RETURN` ends the
program. `GOTO`, `GOSUB`, and `THEN` targets range from 0 to 65535.

## Commands

| Command | Effect |
| --- | --- |
| `LIST` | Prints the stored lines in order as `number text`. |
| `RUN` | Runs the stored program from its lowest line. Variables are kept between runs; loop and subroutine state is cleared. |
| `NEW` | Deletes the stored program. Variables are kept. |

A runtime error stops `RUN` and prints `Error in line N: message`. A jump to a
missing line prints `Error: line N not found`.

## Built-in functions

The build configuration selects which functions exist; see
[development](development.md#build-options).

| Function | Arguments | Result |
| --- | --- | --- |
| `sin(x)`, `cos(x)`, `tan(x)` | 1 | Trigonometry in radians |
| `sqrt(x)` | 1 | Square root; NaN for negative `x` |
| `abs(x)` | 1 | Absolute value |
| `pow(x, y)` | 2 | `x` to the power `y` |
| `log(x)`, `log10(x)` | 1 | Natural and base-10 logarithm; NaN for `x <= 0` |
| `exp(x)` | 1 | `e` to the power `x` |
| `floor(x)`, `ceil(x)` | 1 | Round down or up |
| `min(...)`, `max(...)` | 0 to 8 | Smallest or largest argument; 0 with no arguments |

`min` and `max` are always available. The others require
`G2BASIC_ENABLE_MATH_FUNCTIONS`. Without libm, they use approximations with
the limits described in [development](development.md#approximate-math).

## Not supported

Strings, `INPUT`, `REM`, `LET`, arrays (`DIM`), `ELSE`, `AND`/`OR`/`NOT`,
`^`, `MOD`, `ON ... GOTO`, `STOP`/`CONT`, multiple statements per line, and
saving or loading programs.

## Error messages

Immediate statements return these messages to the host; during `RUN` they are
printed as `Error in line N: message`.

| Message | Cause |
| --- | --- |
| `expected number` | A value was expected, including after an operator or for a string literal. |
| `undefined variable 'name'` | The variable was never assigned, or holds NaN. |
| `unknown function 'name'` | No function with that exact name is registered. |
| `function 'name' expects N arguments, got M` | Wrong argument count. |
| `too many function arguments` | More than 8 arguments. |
| `expression too deeply nested` | More than `G2BASIC_MAX_NESTING` nested parentheses, unary signs, function calls, or `IF ... THEN` statements. |
| `division by zero` | The divisor is zero. |
| `expected '('` / `expected ')'` | Unbalanced parentheses or a malformed call. |
| `Unexpected characters at end` | Extra text after a complete statement, such as `2 ^ 3`. |
| `expected comparison operator` | `IF` without `=`, `<>`, `<`, `>`, `<=`, or `>=`. |
| `expected THEN after IF condition` | `IF` without `THEN`. |
| `GOTO requires a line number`, `GOSUB requires a line number` | The target is not a number. |
| `invalid GOTO line number`, and similar | The target is outside 0 to 65535. |
| `NEXT without matching FOR` | No active loop. |
| `NEXT variable doesn't match FOR variable` | `NEXT` names a different variable than the innermost loop. |
| `RETURN without matching GOSUB` | No active subroutine call. |

A `PRINT` that fails partway has already printed the values before the error.
