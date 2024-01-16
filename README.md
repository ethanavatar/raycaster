# Raycaster

A raycast renderer written in C and Raylib. Inspired by [justinac0/raylib-raycaster](https://github.com/justinac0/raylib-raycaster).

## Building

Requires [Zig v0.11.0](https://ziglang.org).

```bash
# if you use git version 2.13 or later use `--recurse-submodules` instead of `--recursive`
git clone https://github.com/ethanavatar/raycaster --recursive
```

Build for windows:
```bash
zig build -Dtarget=x86_64-windows-msvc -Doptimize=ReleaseSafe run --
```

I made this on Windows, so if you want to build for Linux, changes will have to be made to the `build.zig` file.

![Screenshot](screenshot.png)

