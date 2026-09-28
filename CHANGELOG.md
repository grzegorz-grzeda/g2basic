# Changelog

All notable changes to G2Basic are recorded here. The format is based on
[Keep a Changelog 1.1.0](https://keepachangelog.com/en/1.1.0/), and versions
follow [Semantic Versioning 2.0.0](https://semver.org/spec/v2.0.0.html) as
described in [versioning](docs/versioning.md).

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
