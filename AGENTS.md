# AGENTS.md

## Project
DB Browser for SQLite is a C++ (C++14+) / Qt application built with CMake.

## Build (out-of-source)
Prefer an out-of-source build directory.

```sh
cmake -S . -B build
cmake --build build
```

Resulting binaries are typically under `build/src/` (e.g. `build/src/sqlitebrowser`).

## Unit tests
Unit tests live in `src/tests` and are enabled via `ENABLE_TESTING`.

```sh
cmake -S . -B build-test -DENABLE_TESTING=ON
cmake --build build-test
ctest --test-dir build-test -V
```

## Common CMake options
- `-DENABLE_TESTING=ON`: build unit tests
- `-Dsqlcipher=1`: build with SQLCipher support (if dependencies are available)
- `-DFORCE_INTERNAL_QSCINTILLA=ON`: use bundled QScintilla if system packages cause issues

## Repo conventions (for agents)
- Keep diffs minimal and focused; avoid drive-by refactors and mass reformatting.
- Prefer modifying `src/` over vendored code in `libs/` unless explicitly required.
- For `.ui` files, prefer Qt Designer edits; avoid hand-reformatting generated XML.
- Don’t update `src/translations/*.ts` unless the change is explicitly about translations.
- When changing build behavior, also update `BUILDING.md` (and `README.md` when user-facing).
