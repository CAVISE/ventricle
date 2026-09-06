# Ventricle network simulator

```sh
meson setup build
meson compile -C build ns3-configure
meson compile -C build ns3-build
meson compile -C build
```

`sumo_root` may be omitted when CMake can discover SUMO normally. With the official macOS package,
use `/Library/Frameworks/EclipseSUMO.framework/Versions/Current/EclipseSUMO`.
