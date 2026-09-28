# Repository instructions

These instructions apply throughout this repository. Read the relevant source
and documentation before changing behavior; the implementation is the authority
when documentation and code disagree.

## Project map

G2Basic is a C library (a BASIC interpreter) with an interactive example
program, built with CMake. Start with [architecture](docs/architecture.md) and
[development](docs/development.md). Language behavior is in
[language](docs/language.md) and the host API in [embedding](docs/embedding.md).

G2Basic is also embedded in other projects, such as HomeCore, through
`add_subdirectory()`. Changes must keep working in that mode: embedded defaults
differ, and top-level-only targets such as `docs` must stay guarded.

## Working conventions

- Follow the [C coding standard](docs/coding-standard.md) for new and changed
  code. Keep formatting changes scoped to edited code.
- Keep the public API in `src/g2basic.h` and internal hooks in internal headers.
  Keep the library free of platform dependencies; only `g2basic_math.c` may
  use libm.
- Treat `external/doxygen-awesome-css` as a pinned submodule; make updates
  explicit. Preserve license notices.
- Update the relevant guide when changing language behavior, the API, build
  options, CI, or setup. Keep README concise and link to detailed docs.
- Describe current behavior, including limitations. Do not document features
  the code does not implement.

## Mandatory versioning

G2Basic follows Semantic Versioning 2.0.0 as defined in
[versioning](docs/versioning.md). Every change to shipped code (`src/` and
CMake files that affect the library) MUST, in the same change:

- Increment `VERSION` in the `project(g2basic ...)` call in `CMakeLists.txt`,
  the only version source. While the major version is 0, incompatible
  public-interface changes increment MINOR, and compatible features and fixes
  increment PATCH.
- Add a matching [CHANGELOG.md](CHANGELOG.md) entry, marking incompatible
  changes as **Breaking:**.

The public interface includes the BASIC language, not only `g2basic.h`: a
change that makes existing programs fail or print differently is incompatible.
Documentation, comment-only, test, CI, example, and tooling changes do not
change the version. Never reuse or rewrite a released version.

The completion report MUST state the version decision: the new version and the
reason for the increment, or why the change is exempt.

## Validation

Use the commands in [development](docs/development.md). For code changes:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
python3 tests/check_math.py
```

Then run the smoke test from [development](docs/development.md#tests) and check
the affected language cases by hand. For header changes, run `doxygen Doxyfile`;
it must finish without warnings. For CMake changes, also check an embedded
build, for example HomeCore's.

Report what was checked and what remains unverified. CI builds the Linux
example, runs the smoke test, and builds the API documentation. It does not run
the math tests.
