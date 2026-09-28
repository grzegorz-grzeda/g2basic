# Versioning

G2Basic follows [Semantic Versioning 2.0.0](https://semver.org/spec/v2.0.0.html).
Every change to shipped code carries a version decision, recorded in the same
change. Projects that embed G2Basic, such as HomeCore, rely on the version to
judge the effect of a submodule update.

## Version source

The only version definition is `VERSION` in the `project(g2basic ...)` call in
the root `CMakeLists.txt`. The API documentation header shows it. Never write
the version anywhere else, including file comments.

CMake accepts only numeric versions. SemVer pre-release identifiers and build
metadata (`1.0.0-rc.1`, `1.0.0+abc1234`) appear only in git tags and release
names. The `project()` version is the release they lead to.

## Public interface

SemVer compatibility is judged against this interface:

- The C API in `src/g2basic.h`: declarations and the documented behavior,
  return values, and callback contracts.
- The BASIC language: accepted statements, commands, expressions, and their
  results, as described in the [language reference](language.md). This
  includes the names and argument counts of built-in functions, and PRINT's
  default number format.
- The CMake integration: the `g2basic` target, its include directory, the
  `G2BASIC_*` options, and their standalone and embedded defaults.

Error message wording, `LIST` layout, internal functions and structures,
`g2basic_math.h`, and the example program are not part of the interface.

## Choosing the increment

G2Basic is in initial development (major version 0). SemVer allows anything to
change in 0.y.z; this project narrows that with a fixed convention:

| Change | While 0.y.z | From 1.0.0 |
| --- | --- | --- |
| Incompatible change to the public interface | MINOR (0.1.0 → 0.2.0) | MAJOR |
| Backward-compatible feature, such as a new statement or API function | PATCH (0.1.0 → 0.1.1) | MINOR |
| Backward-compatible bug fix | PATCH | PATCH |

Examples of incompatible changes: removing or renaming a statement or built-in
function, changing a return value of `g2basic_parse()`, changing an option's
default, or making previously valid programs fail or print differently.
Resetting follows SemVer: a MINOR increment resets PATCH to 0, and a MAJOR
increment resets MINOR and PATCH. When one change contains several kinds of
change, apply only the largest increment once. Moving to 1.0.0 is a deliberate
maintainer decision, made when the language and API are considered stable.

## When to change the version

Change the version in the same commit or pull request as a change to shipped
code: `src/`, and CMake files that affect how the library is built or
configured.

These changes do not change the version: documentation, comment-only edits,
tests, CI workflows, the example program, documentation tooling, and
formatting-only changes. A version number is never reused. If a change lands
without its increment, the fix is a follow-up increment, not a rewrite of the
history.

## Recording the change

Each version change adds an entry at the top of [CHANGELOG.md](../CHANGELOG.md):

```markdown
## [0.1.1] - 2026-10-05

### Fixed
- `g2basic_parse()` sets the error message for out-of-range line numbers.
```

Use the sections `Added`, `Changed`, `Deprecated`, `Removed`, `Fixed`, and
`Security`, and omit empty ones. Describe the effect on host programs and BASIC
programs, not the implementation. Mark incompatible changes with **Breaking:**
at the start of the item.

Releases are git tags named `vX.Y.Z` on the commit that sets that version.
Pre-releases use tags such as `v1.0.0-rc.1`. Tags are never moved or deleted.
Publishing a GitHub release for a tag attaches the Linux binary built by CI.
