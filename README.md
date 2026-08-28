# SPACECRAFT DOCKING SIMULATOR

A 2D OpenGL spacecraft docking simulation game developed as a graphics project.

The player controls a spacecraft and attempts to safely approach and dock with a space station. The game includes multiple difficulty levels, spacecraft movement and rotation, velocity control, docking guidance, fuel management, docking conditions, mission scoring, success/failure screens, mouse interaction, and a space-themed visual interface.

---

## 1. DOWNLOAD AND PLAY (FOR NORMAL USERS)

### Windows users — no programming knowledge required

1. Download the latest **`Spacecraft_Docking_Simulator_Windows.zip`** from the GitHub Releases section.
2. Extract the ZIP file.
3. Open the extracted `Spacecraft_Docking_Simulator` folder.
4. Double-click **`Spacecraft_Docking_Simulator.exe`**.
5. The game will start.
6. Choose **Easy**, **Medium**, or **Hard**.
7. Follow the on-screen docking guidance and controls.

**You do not need to install C++, MSYS2, MinGW, FreeGLUT, or any programming software to play the packaged Windows version.**

Keep the `.exe` and `stage11_space_background.bmp` in the same folder.

If Windows displays a security warning for an application downloaded from the internet, verify that the file came from this project's official GitHub repository/release before choosing whether to run it.

---

## 2. GAME OVERVIEW

The objective is to safely maneuver the spacecraft toward the docking station and make a successful connection.

During the mission the player must manage:

- Distance from the docking station
- Approach speed
- Total spacecraft velocity
- Lateral velocity
- Spacecraft orientation / alignment
- Fuel
- Docking-port position
- Braking
- Final docking conditions

The docking system is designed so that docking is not limited to one perfectly straight 180-degree orientation. The spacecraft can use its rear docking port/corridor and maneuver using rotation to approach the station more naturally.

The game also evaluates the quality of the docking and displays mission information such as score, contact speed, lateral speed, alignment, and remaining fuel.

---

## 3. DIFFICULTY LEVELS

### EASY
Designed for learning the controls and basic docking procedure.

### MEDIUM
Uses standard mission conditions and requires more controlled maneuvering.

### HARD
Uses tighter docking requirements and requires more precise control of velocity, alignment, and final approach.

---

## 4. CONTROLS

### Main menu

|    Action     | Keyboard |      Mouse      |
|---------------|----------|-----------------|
|  Select Easy  |    `1`   |    Click Easy   |
| Select Medium |    `2`   |   Click Medium  |
|  Select Hard  |    `3`   |    Click Hard   |
|      Quit     |    `Q`   | Click Quit Game |

The difficulty buttons are interactive and provide visual hover feedback when the mouse is moved over them.

### During the mission

|           Action          |    Control    |
|---------------------------|---------------|
|  Increase forward thrust  |   `UP ARROW`  |
| Reverse / reduce approach |  `DOWN ARROW` |
|  Rotate spacecraft left   |  `LEFT ARROW` |
|  Rotate spacecraft right  | `RIGHT ARROW` |
|           Brake           |    `SPACE`    |
|           Help            |      `H`      |
|       Reset / Retry       |      `R`      |

Use small thrust inputs near the docking station. A controlled final approach is safer than entering the docking corridor at high speed.

### Result screen

|     Action     | Keyboard |    Mouse    |
|----------------|----------|-------------|
|      Retry     |    `R`   | Click Retry |
| Return to menu |    `M`   | Click Menu  |
|      Help      |    `H`   | Click Help  |
|      Quit      |    `Q`   | Click Quit  |

The result-screen buttons provide mouse hover feedback.

---

## 5. DOCKING PROCEDURE

A typical mission can be approached in these steps:

1. Select a difficulty.
2. Observe the spacecraft's distance, velocity, alignment, and fuel.
3. Use forward thrust to begin approaching the station.
4. Use rotation to orient the spacecraft and place the rear docking port toward the docking corridor.
5. Use lateral movement/maneuvering to reduce alignment error.
6. Reduce the approach velocity as the spacecraft gets closer.
7. Use the brake when necessary.
8. Enter the docking corridor under controlled conditions.
9. Maintain a safe final approach speed and low lateral velocity.
10. Make contact with the docking port.
11. The game evaluates the docking conditions and produces a mission result.

The spacecraft has separate rear-side propulsion visuals and a central rear docking corridor so that the propulsion system and docking port are visually distinguishable.

---

## 6. DOCKING CONDITIONS

The simulator evaluates several parameters rather than simply checking whether the spacecraft visually touches the station.

Important parameters include:

- Approach/contact speed
- Lateral speed
- Alignment error
- Docking-port position
- Distance
- Fuel remaining

A docking can therefore fail even when the spacecraft appears close to the station if its velocity or alignment is outside the allowed conditions.

A successful docking produces a mission score and docking-quality information.

---

## 7. GAME INTERFACE

The interface includes:

- Mission status
- Fuel percentage
- Docking distance
- Alignment information
- Approach/total velocity
- Braking indicator
- Docking guidance
- Mission score
- Docking quality
- Success/failure information
- Keyboard shortcuts
- Interactive menu buttons
- Space-themed background

The game uses a custom 2D OpenGL/FreeGLUT interface with a space background image.

---

## 8. REQUIREMENTS FOR BUILDING FROM SOURCE

The ready-to-play Windows release does **not** require these tools.

They are only required if you want to compile the source code yourself.

### Development environment

- Windows 10 or Windows 11
- MSYS2
- MinGW-w64 / MinGW64 GCC
- FreeGLUT
- OpenGL
- GLU

The project is written in C++ and uses OpenGL/FreeGLUT for graphics and window/input handling.

---

## 9. BUILD FROM SOURCE USING MSYS2 MINGW64

Open the **MSYS2 MinGW64** terminal and go to the directory containing the source file.

Example:

```bash
cd /c/Users/YOUR_USERNAME/Downloads
```

Compile:

```bash
g++ spacecraft_docking_stage11.cpp -o spacecraft_docking_stage11.exe -lfreeglut -lopengl32 -lglu32
```

If compilation succeeds, the executable will be created in the same directory.

Run it with:

```bash
./spacecraft_docking_stage11.exe
```

### Important

The runtime background file must be available to the executable:

```text
stage11_space_background.bmp
```

For the easiest setup, keep the BMP in the same directory as the executable.

---

## 10. PROJECT GOALS

The project demonstrates concepts including:

- 2D computer graphics
- OpenGL rendering
- FreeGLUT window and input handling
- Interactive graphical user interfaces
- Keyboard and mouse interaction
- Spacecraft movement
- Rotation and maneuvering
- Velocity-based movement
- Braking
- Docking-port positioning
- Collision/contact detection
- Docking-condition evaluation
- Difficulty levels
- Mission scoring
- Game-state management
- Success/failure handling
- Visual guidance systems
- Custom space-themed interface design

---
