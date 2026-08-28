# SPACECRAFT Docking Simulator

A Graphics Lab project: an interactive spacecraft docking simulator built with C++ and OpenGL/FreeGLUT.

## Download and Play — Windows

Download the latest **Windows ZIP** from Releases, extract it, and double-click `Spacecraft_Docking_Simulator.exe`. No C++ compiler, MSYS2, MinGW, FreeGLUT, or graphics-development software is required for the packaged version.

Keep `stage11_space_background.bmp` beside the executable.

## What the Game Does

The player pilots a spacecraft toward an orbital space station and attempts a controlled docking. Features include:

- Easy, Medium, and Hard difficulty levels
- Thrust, braking, rotation, and maneuvering
- Velocity, lateral-speed, and alignment monitoring
- Docking-port/contact detection
- Fuel management
- Docking success/failure evaluation
- Mission scoring and result screens
- Keyboard and mouse controls
- In-game help
- Interactive futuristic UI
- Space-themed graphics and background

## Controls

### Main Menu
- `1` — Easy
- `2` — Medium
- `3` — Hard
- `Q` — Quit
- Mouse — click buttons

### Mission
Use the on-screen controls. `H` opens Help and `R` can be used for retry/reset where enabled.

### Result Screen
- `R` — Retry
- `M` — Menu
- `H` — Help
- `Q` — Quit
- Mouse — click buttons

## Project Structure

```text
SPACECRAFT-Docking-Simulator/
├── README.md
├── src/
│   └── spacecraft_docking_stage11.cpp
├── assets/
│   └── stage11_space_background.bmp
├── screenshots/
│   ├── main_menu.png
│   ├── gameplay.png
│   ├── docking_success.png
│   └── docking_failure.png
└── release/
    └── Spacecraft_Docking_Simulator_Windows.zip
```

Replace placeholder files with your final files.

## Build From Source

### Requirements

- Windows 10/11
- MSYS2
- MinGW-w64 / MinGW64 GCC
- FreeGLUT
- OpenGL / GLU

### Compile

Open **MSYS2 MinGW64** in the source directory:

```bash
g++ spacecraft_docking_stage11.cpp -o Spacecraft_Docking_Simulator.exe -lfreeglut -lopengl32 -lglu32
```

Run:

```bash
./Spacecraft_Docking_Simulator.exe
```

The background asset must be named `stage11_space_background.bmp` and be available to the executable.

## Portable Windows Release

The intended player package is:

```text
Spacecraft_Docking_Simulator/
├── Spacecraft_Docking_Simulator.exe
├── libfreeglut.dll
└── stage11_space_background.bmp
```

Test the ZIP outside the MSYS2 development environment before publishing it. Use `ldd Spacecraft_Docking_Simulator.exe` to identify additional non-system runtime dependencies if any appear. Do not redistribute Windows system DLLs.

## Preview

Add screenshots or a GIF here:

![Main Menu](screenshots/main_menu.png)

![Gameplay](screenshots/gameplay.png)

![Successful Docking](screenshots/docking_success.png)

![Failed Docking](screenshots/docking_failure.png)

## Academic Project

This project demonstrates 2D computer graphics, OpenGL rendering, FreeGLUT window/input handling, interactive GUI design, spacecraft movement and rotation, velocity-based simulation, contact detection, game-state management, docking evaluation, and visual feedback.

Created as a Graphics Lab course project.
