# Development

Run all commands from the repository root. Follow the
[C coding standard](coding-standard.md) for new and changed code.

## Setup

Required: CMake 3.15 or later, a C compiler, and Make or Ninja. The math tests
also need Python 3 and `nm`. API documentation needs Doxygen 1.9.8 or later and
Graphviz. On Ubuntu:

```bash
sudo apt-get install build-essential cmake python3 doxygen graphviz
git submodule update --init --recursive
```

The only submodule is the documentation theme; the library builds without it.

## Building

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/examples/interactive/g2basic-interactive
```

The interactive interpreter reads lines from standard input and exits at end of
input (Ctrl-D). It accepts piped programs:

```bash
printf '10 FOR I = 1 TO 3\n20 PRINT I\n30 NEXT I\nRUN\n' \
  | ./build/examples/interactive/g2basic-interactive
```

## Build options

| Option | Standalone default | Embedded default | Effect |
| --- | --- | --- | --- |
| `G2BASIC_BUILD_EXAMPLES` | ON | OFF | Build `examples/interactive` |
| `G2BASIC_ENABLE_MATH_FUNCTIONS` | ON | OFF | Register the 11 math functions besides `min` and `max` |
| `G2BASIC_ENABLE_MATH` | ON | OFF | Implement them with libm instead of approximations |

"Embedded" means added to another project with `add_subdirectory()`. CMake
caches option values, so an existing build directory keeps earlier choices.
Exactly one backend source is compiled:

| `..._MATH_FUNCTIONS` | `..._MATH` | Source | Built-ins | libm |
| --- | --- | --- | --- | --- |
| ON | ON | `g2basic_math.c` | 13 | linked |
| ON | OFF | `g2basic_math_crude.c` | 13, approximated | not linked |
| OFF | either | `g2basic_math_none.c` | `min`, `max` | not linked |

### Approximate math

`g2basic_math_crude.c` uses polynomial and series approximations with bounded
range reduction. It is meant for simple embedded calculations, not libm
accuracy or full IEEE special-case behavior.

- Angles are radians. Trigonometric inputs above 1,000,000 radians in magnitude
  return NaN.
- `tan` returns NaN when the approximated cosine is below 0.00001 in magnitude,
  and accuracy degrades near its poles.
- Invalid domains return NaN. Exponential overflow returns infinity and
  underflow returns zero.
- A negative base supports integer exponents below 2^53 in magnitude; other
  exponents return NaN.

The source still includes `<math.h>` for constants and classification macros,
which do not need libm.

## Tests

```bash
python3 tests/check_math.py
```

The script compiles both math backends as shared libraries with
`-Wall -Wextra -Werror` and compares them with Python's `math` module. It
checks domains and extremes, and verifies that the approximation backend does
not reference libm symbols.

There are no unit tests for the interpreter itself. CI runs a Fibonacci program
through the interactive example as a smoke test; run the same check locally:

```bash
printf '10 n = 10\n20 x = 0\n30 y = 1\n40 FOR i = 1 TO n\n50 z = x + y\n60 x = y\n70 y = z\n80 PRINT z\n90 NEXT i\nRUN\n' \
  | ./build/examples/interactive/g2basic-interactive
```

It should print 1, 2, 3, 5, 8, 13, 21, 34, 55, and 89. The smoke test does not
check its own output. For behavior changes, also check the affected cases in
the [language reference](language.md) by hand.

## Formatting

`.clang-format` is based on the Chromium style with four-space indentation. It
also contains `LineWidth: 120`, which is not a clang-format option, so
clang-format currently rejects the file. The code follows the Chromium
default 80-column limit. Until the file is fixed, check changed code with a
temporary copy of the configuration without that line.

## API documentation

The public header `src/g2basic.h` and the internal hook `src/g2basic_math.h`
carry Doxygen comments, grouped as follows:

| Group | Contents |
| --- | --- |
| Embedding API (`api`) | Setup and output (`api_setup`), custom functions (`api_functions`), execution (`api_execution`) |
| Internal interfaces (`internals`) | Built-in function backends (`math_backends`) |

The main page, the `internals` group, and the example program are defined in
`docs/doxygen/groups.dox`. Generate the HTML from the repository root:

```bash
git submodule update --init external/doxygen-awesome-css
doxygen Doxyfile
```

Open `build/docs/index.html`. Set `G2BASIC_DOCS_VERSION` to show a version in
the page header, for example `G2BASIC_DOCS_VERSION=0.1.0 doxygen Doxyfile`.
In a standalone CMake build directory, `cmake --build build --target docs`
writes to `build/docs` and stamps the `project()` version. The target is
disabled, with a message, when Doxygen, `dot`, or the theme is missing, and it
is never defined when G2Basic is embedded in another project.

Undocumented declarations or parameters and malformed comments are warnings,
and `WARN_AS_ERROR = FAIL_ON_WARNINGS` makes the run fail on them. When adding
or changing a declaration in a header:

- Put it inside its group's `@defgroup ... @{ ... @}` block.
- Give it a `@brief` and document every parameter, the return values, pointer
  ownership and lifetime, and whether `NULL` is allowed.
- Describe current behavior, including limitations. Update the
  [language reference](language.md) or [embedding](embedding.md) guide when
  behavior visible to BASIC programs or hosts changes.
- In Doxygen 1.9.8, a backtick code span containing a single quote breaks
  parsing of the following commands. Use double quotes around such text instead.

### Theme

The site uses [doxygen-awesome-css](https://github.com/jothepro/doxygen-awesome-css)
v2.5.0, pinned as the `external/doxygen-awesome-css` submodule (MIT license).
It supports Doxygen 1.9.6 to 1.18.0 and requires `HTML_COLORSTYLE = LIGHT`. It
provides the responsive layout, a light/dark toggle, and inverted Graphviz
graphs in dark mode.

- `docs/doxygen/g2basic.css` sets the teal palette shared with HomeCore. Keep
  link text and text on the primary colour at a WCAG contrast of at least
  4.5:1; the file records the current ratios.
- `docs/doxygen/header.html` is Doxygen 1.9.8's default header, generated with
  `doxygen -w html header.html footer.html style.css`, plus the toggle script.
  It also renders correctly with Doxygen 1.12. When CI's Doxygen version
  changes, regenerate it the same way and re-add the two script tags.

## CI

Two GitHub Actions workflows run on pull requests and pushes to `main`.

**Build Linux Executable** (`.github/workflows/build-linux.yml`) also runs on
pushes to `develop` and on published releases. It builds the standalone Release
configuration on `ubuntu-latest`, runs the Fibonacci smoke test, and uploads
the interactive binary as the `g2basic-interactive-linux-x64` artifact for 30
days. For a published release it attaches
`g2basic-interactive-linux-x64.tar.gz` with the binary, README, and license.
The math tests are not run in CI.

**API documentation** (`.github/workflows/docs.yml`) fetches only the theme
submodule, installs Doxygen and Graphviz, and runs `doxygen Doxyfile` with the
`project()` version and short commit as `G2BASIC_DOCS_VERSION`. It fails on any
documentation warning and uploads the HTML as a `github-pages` artifact, which
pull-request runs keep as a downloadable preview. On pushes to `main`,
`Publish API documentation` deploys it to
[GitHub Pages](https://grzegorz-grzeda.github.io/g2basic/). Pages must be
enabled once under repository **Settings → Pages → Build and deployment →
Source: GitHub Actions**.
