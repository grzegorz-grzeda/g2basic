# Changelog

All notable changes to G2Basic are recorded here. The format is based on
[Keep a Changelog 1.1.0](https://keepachangelog.com/en/1.1.0/), and versions
follow [Semantic Versioning 2.0.0](https://semver.org/spec/v2.0.0.html) as
described in [versioning](docs/versioning.md).

## [0.1.1] - 2026-09-29

### Fixed
- Deeply nested expressions no longer overflow the C stack. Nesting of
  parentheses, unary signs, function calls, and `IF ... THEN` statements is
  limited per line, and deeper lines fail with `expression too deeply nested`.
  Previously each level recursed without bound, so a single long line could
  corrupt memory on a microcontroller.

### Added
- `G2BASIC_MAX_NESTING` CMake option for that limit: 32 in a standalone build,
  8 by default when embedded.
- `tests/check_nesting.py`, run in the Linux CI workflow.

## [0.1.0] - Baseline

Baseline for this changelog, when the versioning policy was adopted. Earlier
changes are recorded only in the git history. This version provides:

- Line-numbered programs with `LIST`, `RUN`, and `NEW`, and immediate
  statements.
- Assignments, arithmetic expressions, `PRINT`, `IF ... THEN`, `FOR ... NEXT`
  with `STEP`, `GOTO`, `GOSUB` and `RETURN`, and `END`.
- Custom C functions, and built-in math functions backed by libm,
  approximations, or omitted, selected by CMake options.
- Host output callbacks for text and numbers.
