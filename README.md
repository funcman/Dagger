Dagger
===

A classic 2d game engine for Diablo like MMOARPG.

Actually, the technology of this code is from jx-sword2 and xlqy-blade.

Build
===

Requirements: CMake 3.16+, and a 64-bit x86 toolchain (MSVC on Windows,
GCC/Clang elsewhere). The engine contains hand-written x86-64 assembly
(`Engine/asm/`), so only x86-64 is supported.

Windows (PowerShell):

```
scripts\build.ps1                # Release build
scripts\build.ps1 -Config Debug  # Debug build
scripts\build.ps1 -Clean         # clean and rebuild
```

Linux / macOS:

```
scripts/build.sh                 # Release build
scripts/build.sh Debug           # Debug build
scripts/build.sh --clean         # clean and rebuild
```

Or invoke CMake directly:

```
cmake -S . -B build
cmake --build build
```

Artifacts are written under `build/`:

- `bin/` — `Client` executable, `DBase` and `DEngine` shared libraries
- `lib/` — import / static libraries

The first build also initializes the git submodule `3rd/SDL3`.
