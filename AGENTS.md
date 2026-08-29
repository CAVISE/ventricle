# Ventricle development guidelines

## Project overview

Ventricle is a C++20 network simulator built with Meson. The source tree follows ns-3's module layout:

- `src/core/` contains code shared by the simulator and extension modules.
- `src/extensions/<module>/model/` contains the module's runtime implementation.
- `src/extensions/<module>/helpers/` contains setup helpers, adapters, and conversion utilities.
- `src/extensions/<module>/tests/` contains module-local tests.
- `subprojects/` contains subprojects that cannot be provided by other means.

Keep new functionality in the narrowest applicable module. Put reusable simulator-wide utilities in
`src/core`; do not make core depend on an extension.

## Build and test

Set up a new build directory and build ns-3 before compiling Ventricle:

```sh
meson setup build
meson compile -C build ns3-configure
meson compile -C build ns3-build
meson compile -C build
```

After the initial ns-3 build, ordinary source changes normally require only:

```sh
meson compile -C build
meson test -C build --print-errorlogs
```

After changing Meson files, wraps, or CMake options, reconfigure explicitly when Meson does not do so
automatically:

```sh
meson setup --reconfigure build
```

Run the smallest relevant test while iterating, then run the complete suite before handing off a
change. Keep the build warning-free where practical, but do not edit generated Vanetza ASN.1 sources
to address upstream warnings.

## C++ style

- Use C++20 and format C/C++ sources with the root `.clang-format` file.
- Write declarations and statements without manual line wrapping; only keep line breaks introduced
  by `clang-format`.
- Use `#pragma once` in headers.
- Include project headers first (`"src/..."`), then standard/ns-3/Vanetza headers as sorted by
  clang-format.
- Put code in `vcle` or a nested module namespace such as `vcle::itsg5`
- Follow the established naming: PascalCase for types; lower camel case for functions, local
  variables, and parameters; and lower camel case with a trailing underscore for data members
  (`someVar_`).
- Add brief documentation for public types and operations. Comments should explain intent or a
  non-obvious constraint.
- When overriding a virtual method(s), group them together and write a comment in format /* <BaseClass> implementation*/.
- Prefer `using namespace` in source (.cpp) files.
- Split class fields and methods in single scope (private, protected), by duplicate scope mark: `private:` or `protected:`.

Format project sources (excluding downloaded subprojects) with:

```sh
find src -type f \( -name '*.cpp' -o -name '*.hpp' \) -print0 | xargs -0 clang-format -i
```

## Meson organization

- ...

## Tests

- ...

## Wrapped dependencies

- ...
