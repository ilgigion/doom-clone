# Doom Clone

Simple Doom-like engine written in C++ using SDL2.

## Build Instructions

Windows build requirements: CMake, Ninja, MinGW-w64 x64, SDL2 and SDL2_mixer
development packages for MinGW (not SDL3 or Visual C++ packages).

The default library locations are:

- `D:/Dev/Library/SDL2-2.32.10/x86_64-w64-mingw32`
- `D:/Dev/Library/SDL2_mixer-2.8.2/x86_64-w64-mingw32`

Official downloads:

- https://github.com/libsdl-org/SDL/releases/tag/release-2.32.10
- https://github.com/libsdl-org/SDL_mixer/releases/tag/release-2.8.2

From the project root, run in PowerShell:

```powershell
& 'C:/Program Files/CMake/bin/cmake.exe' -S . -B build/local -G Ninja `
  -DCMAKE_MAKE_PROGRAM=C:/msys64/mingw64/bin/ninja.exe `
  -DCMAKE_CXX_COMPILER=C:/msys64/mingw64/bin/g++.exe `
  -DCMAKE_BUILD_TYPE=Debug
& 'C:/Program Files/CMake/bin/cmake.exe' --build build/local --parallel 2
& ./build/local/DoomClone.exe
```

Adjust the tool paths for your installation. To use another library directory,
pass `-DSDL2_PATH=...` and `-DSDL2_MIXER_PATH=...` when configuring.
Use a fresh build directory if an old CMake cache refers to another project path.

The build copies SDL DLLs and assets beside the executables. Launch from the
project root or `build/local` so the relative `assets/` paths resolve.
Keep `C:/msys64/mingw64/bin` on PATH for the MinGW runtime DLLs.

Run the existing test executable from the project root:

```powershell
& ./build/local/DoomClone_tests.exe
```

Or run the complete registered suite (including isolated initialization failures):

```powershell
& 'C:/Program Files/CMake/bin/ctest.exe' --test-dir build/local --output-on-failure
```

The main suite requires a working accelerated SDL video backend. Failure to create
one fails the tests instead of silently skipping graphics checks. SDL video and
renderer failure tests use isolated processes and explicitly selected drivers.

Renderer initialization errors now stop startup with a diagnostic and exit code 1.
Texture loaders preserve an existing texture when its replacement cannot be loaded.
Their existing return types are unchanged: use `.value_or(false)` to check a
`std::optional<bool>` result; testing only the optional's presence is insufficient.

Project decisions and verified progress are tracked in `PROJECT_MEMORY.md`;
remaining audit and floor-rendering stages are in `IMPLEMENTATION_PLAN.md`.

## VS Code

Open the project folder and use CMake Tools to configure and run the `DoomClone`
target. The checked-in `.vscode/settings.json` uses `build/local` and sets the
debug working directory to the project root. This avoids the obsolete cache in
`build`, which was generated before the project was moved.

After changing settings, run `CMake: Configure` from the Command Palette, select
`DoomClone` as the launch target, then launch it through CMake Tools. Compiling
`src/main.cpp` alone does not include the other source files or SDL link settings.

## Projection conventions

`include/FrameProjection.h` contains the SDL-independent frame geometry shared
by walls, floor and sprites. World units are map cells;
direction zero points along +X, positive angles turn towards +Y.
Columns retain the existing equal angular spacing and repeated float addition.
The horizon is `height / 2 + int(bobOffset)` (integer division), and a unit wall
has projected height `height / perpendicularDepth`. Camera height is 0.5 cells.

For a floor sample at continuous screen coordinate `screenY`, forward depth is
`height * 0.5 / (screenY - horizon)`. Pass `row + 0.5` for pixel centres.
`floorPoint` uses the same ray angle as the corresponding wall column.
No finite forward intersection returns `nullopt`. The floor renderer applies
map bounds and a forward depth limit of 30 cells.
Texture coordinates repeat once per cell and support negative world positions.

The floor repeats in world space using an opaque ARGB8888 CPU image and a
streaming SDL texture. Failed replacements retain the previous image; a missing
texture uses a grey background. The ceiling remains a stretched background.
Sprites use perpendicular depth, with their bottom anchored to the floor.
Walls and nearer sprites hide enemy health bars.
Each wall texture keeps its own dimensions, including when used as a fallback.
The damage overlay fades with the supplied alpha and preserves SDL draw state.
HUD randomness uses a separate generator so rendering cannot change gameplay's
random sequence for spread and enemy spawning.

## Simulation

Gameplay advances in fixed 1/120-second steps. Each frame contributes at most
0.1 seconds; longer stalls discard excess time to avoid a catch-up spiral.
The menu pauses simulation. Forward speed is 4.8 cells/s, reverse speed 2.4 cells/s,
acceleration and braking are 28.8 cells/s², and turning is 2.16 radians/s.
These rates approximate the former double player update at 60 FPS. Weapon,
damage, attack and spawn timers now advance with simulation time. Movement and
spawn checks account for the entity's circular collision radius.

The `simulation` CTest group compares 30/60/144 FPS, braking, rotation, pause,
spawn positions and the population cap. Rendering groups check actual pixels
and save BMP previews next to the test executable.

The pure math tests require no SDL window:

```powershell
& 'C:/Program Files/CMake/bin/cmake.exe' --build build/local --target DoomClone_projection_tests
& 'C:/Program Files/CMake/bin/ctest.exe' --test-dir build/local -R '^projection_math$' --output-on-failure
```
