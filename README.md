# G2Basic

[![Build Linux Executable](https://github.com/grzegorz-grzeda/g2basic/actions/workflows/build-linux.yml/badge.svg)](https://github.com/grzegorz-grzeda/g2basic/actions/workflows/build-linux.yml)
[![API documentation](https://github.com/grzegorz-grzeda/g2basic/actions/workflows/docs.yml/badge.svg)](https://grzegorz-grzeda.github.io/g2basic/)

G2Basic is a small BASIC interpreter written in C for microcontrollers and other
embedded hosts. A host passes it one line at a time: numbered lines build a
stored program, and other lines run immediately. All values are numbers.

- Line-numbered programs with `LIST`, `RUN`, and `NEW`
- Assignments, arithmetic expressions, and `PRINT`
- `IF ... THEN`, `FOR ... NEXT` with `STEP`, `GOTO`, `GOSUB` and `RETURN`, `END`
- Custom C functions callable from BASIC, plus optional math built-ins, with or
  without libm
- Heap-allocated state with no fixed limits on program size or nesting
- Output through host callbacks

## Quick start

```bash
git clone https://github.com/grzegorz-grzeda/g2basic.git
cd g2basic
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/examples/interactive/g2basic-interactive
```

Then enter a program and run it:

```basic
10 FOR I = 1 TO 5
20 PRINT I, I * I
30 NEXT I
RUN
```

Each [CI run](https://github.com/grzegorz-grzeda/g2basic/actions) uploads a
pre-built Linux binary, and published releases get one attached.

## Using as a library

```cmake
add_subdirectory(external/g2basic)
target_link_libraries(your_app PRIVATE g2basic)
```

```c
#include "g2basic.h"

g2basic_init(print_text);  // void print_text(const char* text)
g2basic_parse("PRINT 6 * 7", &result, &error);
```

When embedded, examples and the math built-ins other than `min` and `max` are
off by default. See [embedding](docs/embedding.md) for options, the API
contract, and constraints for embedded hosts.

## Documentation

- [API reference](https://grzegorz-grzeda.github.io/g2basic/): grouped HTML docs
  of the headers, published from `main` by CI.
- [Language reference](docs/language.md): statements, expressions, commands,
  built-in functions, error messages, and limits.
- [Embedding](docs/embedding.md): CMake integration, output, custom functions,
  and the API contract.
- [Architecture](docs/architecture.md): parser, program store, execution,
  memory, and known issues.
- [Development](docs/development.md): builds, options, tests, formatting, API
  documentation, and CI.
- [C coding standard](docs/coding-standard.md): rules for new and changed code.
- [Versioning](docs/versioning.md) and [changelog](CHANGELOG.md): SemVer 2.0.0
  rules and release history.
- [Contributor and agent instructions](AGENTS.md) ([CLAUDE.md](CLAUDE.md)
  imports them for Claude Code).

## License

G2Basic is MIT licensed; see [LICENSE](LICENSE). Created by Grzegorz Grzęda.
The documentation theme in `external/doxygen-awesome-css` has its own MIT license.
