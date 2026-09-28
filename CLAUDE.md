# CLAUDE.md

The repository rules live in AGENTS.md and are imported here.

@AGENTS.md

## Quick reference

G2Basic is a numbers-only BASIC interpreter library for embedded hosts. It keeps
one global interpreter state and has no strings, `INPUT`, or `REM`. Do not
document features the code does not implement.

| Task | Guide |
| --- | --- |
| BASIC statements, expressions, errors | [docs/language.md](docs/language.md) |
| Host API, callbacks, custom functions | [docs/embedding.md](docs/embedding.md) |
| Parser, runtime state, known issues | [docs/architecture.md](docs/architecture.md) |
| Builds, options, tests, CI, API docs | [docs/development.md](docs/development.md) |
| Coding rules | [docs/coding-standard.md](docs/coding-standard.md) |

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build
./build/examples/interactive/g2basic-interactive   # REPL; pipe a program in
python3 tests/check_math.py                        # math backend checks
doxygen Doxyfile                                   # API docs -> build/docs; fails on warnings
```

## Notes for Claude Code

- `.clang-format` contains an invalid key (`LineWidth`), so clang-format rejects
  it. Check formatting with a temporary copy without that line.
- An immediate line starting with a digit is stored as a program line. Test
  expressions with `PRINT 2 + 3`, not `2 + 3`.
- `g2basic.c` does not build with `-Werror`; CMake applies `-Werror` only to the
  example. Compile the library without `-Werror` when testing by hand.
- This repository is often checked out as HomeCore's `external/g2basic`
  submodule. Commit here first, then update the submodule pointer there.
