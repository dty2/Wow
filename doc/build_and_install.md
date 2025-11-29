# Build And Install

## Dependence

```bash
# Arch dependence
pacman -S pkgconf wayland wayland-protocols wlr-protocols mesa ffmpeg nlohmann-json tomlplusplus

# 其他平台
# TODO:
```

## Build Command

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```
