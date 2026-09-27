# G2Basic

[![Build Linux Executable](https://github.com/grzegorz-grzeda/g2basic/actions/workflows/build-linux.yml/badge.svg)](https://github.com/grzegorz-grzeda/g2basic/actions/workflows/build-linux.yml)
Simple BASIC interpreter for microcontrollers with dynamic memory management and comprehensive language support.

## Features

- **Dynamic Memory Management**: All data structures use linked lists for unlimited nesting
- **BASIC Language Support**: Variables, functions, control flow (FOR/NEXT, IF/THEN, GOTO, GOSUB/RETURN)
- **Mathematical Functions**: Built-in math library with common functions
- **Line-based Programming**: Traditional BASIC line number support
- **Configurable Output**: Customizable print function for different environments

## Quick Start

### Download Pre-built Binaries

Visit the [Releases](../../releases) page or check the [Actions](../../actions) tab for the latest builds.

### Building from Source

```bash
# Clone the repository
git clone https://github.com/grzegorz-grzeda/g2basic.git
cd g2basic

# Build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Run the interactive interpreter
./examples/interactive/g2basic-interactive
```

### Using as a Library

Add this repository to your project (for example, under `external/g2basic`), then
include it in your main `CMakeLists.txt`:

```cmake
add_subdirectory(external/g2basic)
target_link_libraries(your_app PRIVATE g2basic)
```

The `g2basic` target provides its public include directory, so your code can use
`#include "g2basic.h"`.

Examples are built by default for standalone builds and disabled by default when
included in another project. Override this with `G2BASIC_BUILD_EXAMPLES`:

```bash
# Build only the library
cmake -S . -B build -DG2BASIC_BUILD_EXAMPLES=OFF
cmake --build build
```

Set the option to `ON` to enable examples explicitly. If reusing an existing build
directory, CMake retains the cached option value.

### Optional Math Functions

`G2BASIC_ENABLE_MATH_FUNCTIONS` defaults to `ON`. Set it to `OFF` to
register no built-in math functions and omit libm, regardless of the backend
selection. Arithmetic, comparisons, control flow, and custom function
registration still work.

| Math functions | Use libm | Implementation |
| --- | --- | --- |
| ON | ON | `g2basic_math.c` (libm) |
| ON | OFF | `g2basic_math_crude.c` (approximations) |
| OFF | Either | `g2basic_math_none.c` (no built-ins) |

```bash
cmake -S . -B build -DG2BASIC_ENABLE_MATH_FUNCTIONS=OFF
```

`G2BASIC_ENABLE_MATH` selects libm-backed built-ins (`ON`) or small
approximations with no libm linkage (`OFF`). It defaults to `ON` standalone
and `OFF` when embedded. CMake caches the setting:

```bash
cmake -S . -B build -DG2BASIC_ENABLE_MATH=OFF
```

With math functions enabled, both backends register `sin`, `cos`, `tan`, `sqrt`, `abs`, `pow`, `log`,
`log10`, `exp`, `floor`, `ceil`, `min`, and `max`. Custom function
registration remains available. When math functions are enabled, CMake compiles exactly one implementation:
`src/g2basic_math.c` for `ON`, or `src/g2basic_math_crude.c` for `OFF`. Only
the libm implementation links `libm`.

The fallback uses polynomial/series approximations and bounded range reduction.
Angles are radians; trigonometric inputs above 1,000,000 radians in magnitude
return NaN. Tangent returns NaN when the approximated cosine is below 0.00001
in magnitude. Accuracy degrades near tangent poles. Invalid domains return NaN;
exponential overflow/underflow returns infinity/zero. Negative-base powers
support integer exponents with magnitude below 2^53; larger exponents return
NaN. These routines are intended for simple embedded calculations, not
libm-level accuracy or complete IEEE special-case compatibility.

Direct source builds must compile `src/g2basic.c` and exactly one of
`src/g2basic_math.c` (link libm), `src/g2basic_math_crude.c` (no libm),
or `src/g2basic_math_none.c` (no built-ins or libm).
No preprocessor definition is needed to select the implementation. The fallback still includes `<math.h>` for constants and
classification macros, which do not require libm.

### Example Usage

```basic
10 PRINT "Hello, World!"
20 FOR I = 1 TO 10
30 PRINT "Count: " I
40 NEXT I
50 END
```

## CI/CD

This project includes automated builds for Linux. See [CI Documentation](.github/CI_README.md) for details.