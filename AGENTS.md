# Ventricle development guidelines

Important: changes here should be made only manually. You suggest user what to add here,
but insertions should be done strictly manually.

## Project overview

Ventricle is a C++23 network simulator built with Meson. The source tree follows ns-3's module layout:

- `ventricle/core/` contains code shared by the simulator and extension modules.
- `ventricle/extensions/<module>/model/` contains the module's runtime implementation.
- `ventricle/extensions/<module>/helpers/` contains setup helpers, adapters, and conversion utilities.
- `ventricle/extensions/<module>/tests/` contains module-local tests.
- `subprojects/` contains subprojects that cannot be provided by other means.

Keep new functionality in the narrowest applicable module. Put reusable simulator-wide utilities in
`ventricle/core`; do not make core depend on an extension.

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

- Use C++23 and format C/C++ sources with the root `.clang-format` file.
- Prefer `const std::string&` over `std::string_view` for read-only string parameters and borrowed access to existing strings throughout handwritten project code. Use owning `std::string` values when ownership or a newly constructed result is required.
- For all Abseil error helpers (`absl::*Error`), use plain string literal messages starting with a lowercase letter; do not concatenate, format, or otherwise manipulate strings in error construction calls.
- Within function bodies, group statements by purpose and separate those groups with blank lines to improve readability.
- Write declarations and statements without manual line wrapping; only keep line breaks introduced
  by `clang-format`.
- Use `#pragma once` in headers.
- In header files, always use angle brackets (`#include <...>`) for all includes, including project headers; never use quoted includes (`#include "..."`).
- All project include paths must start with `ventricle/` (for example, `#include <ventricle/core/...>`).
- Include project headers first (`ventricle/...`), then standard/ns-3/Vanetza headers as sorted by
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
find ventricle -type f \( -name '*.cpp' -o -name '*.hpp' \) -print0 | xargs -0 clang-format -i
```

## Documenting code

Use doxygen documenting style with doxygen commands starting with @. Use references in documentation
when mentioning the existing class/function. Use the follow (java-doc) style for
multi-line comments:

```
/**
 * @brief doc strings here
 */
```

For short doc strings, either omit them entirely or use triple '/'.

What code to document:

- Documentation only exists in header files. In source files, you are allowed to leave comments to
  reason the certain behavior.
- You should always document classes and enums. For class, the documentation should consist of: brief block describing
  the class in a single sentence, extended documentation block (if needed) stating the functionality, purpose and limitations
  of current class. Then, in the second paragraph list important methods or provide code snippet explaining how to use this class.
  If class is simple, only @brief block is enough. For enums, document all enum values and briefly describe the enum.
- Methods and functions are documented as needed. Do not document simple constructors or simple accessors or getters:
  if their contract is clear to consumer code, leave them undocumented.
- Macros are always documented with their purpose and reasoning for their usage.
- For header files, if file is located under core/ or is a large header file for any directory in extensions/,
  then you should write @file block describing the model/core component designation, providing optional small cpp
  sample and describing limitation, assumptions and unresolved problems (if any) for the provided functionality.

The code you should document is as follows:

- If function arguments are not obvious from the function name and argument names combined, use @param with
  \[in\], \[out\] tags to explain what are those arguments and how they affect the function behavior.
- Prefer not to copy over the same information between documentation block, but use @see to reference an already
  documented class or function.


## Meson organization

- ...

## Tests

- Use GoogleTest for C++ tests and pytest for Python integration tests.
- Put `TEST`, `TEST_F`, and `TEST_P` declarations at file scope; reserve namespaces for test fixtures and their helpers.
- Link GoogleTest only into test targets. Tests using the ns-3 simulator must clean up simulator state between cases.

### Test suites

- `large`: Tests that require substantial setup or additional system binaries. These tests are generally written in Python.
- `medium`: Normal tests that require mocks, functionality from other components, or little system setup, such as a configuration file.
- `light`: Small unit tests testing a single function or a small class.

### Test priorities

- `critical`: Tests that must be run to ensure the simulator operates correctly.
- `normal`: Tests that should be run.
- `optional`: Tests that are not strictly required.

Each test must have at least one suite (`large`, `medium`, or `light`) and one priority (`critical`, `normal`, or `optional`).

## Wrapped dependencies

- ...
