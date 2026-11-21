# Ventricle network simulator

```sh
meson setup build
meson compile -C build ns3-configure
meson compile -C build ns3-build
meson compile -C build
```

The configuration extension provides scoped object identifiers and JSON Schema generation
for explicitly exposed ns-3 types. It uses Glaze 9.0.0, downloaded through Meson when needed.
GoogleTest 1.11 or newer is required for the C++ tests.

To include the Python schema tests, install pytest and enable integration tests:

```sh
meson setup --reconfigure build -Dintegration_tests=enabled
meson compile -C build
meson test -C build --print-errorlogs
```
