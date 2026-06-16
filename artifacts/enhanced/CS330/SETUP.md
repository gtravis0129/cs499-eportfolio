# CS330Project — VS 2022 Setup (vcpkg auto-managed)

Your project is restructured so Visual Studio 2022 installs GLEW, GLFW, and GLM
**automatically** the first time you build. No manual .lib / .dll juggling.

## Folder layout
```
CS330Project/
├─ CS330Project.sln              <- open this
├─ vcpkg.json                    <- declares glew, glfw3, glm (auto-installed)
├─ CS330Project/
│   └─ CS330Project.vcxproj      <- project file (vcpkg manifest mode ON)
├─ Source/                       <- MainCode, SceneManager, ViewManager
├─ Utilities/                    <- ShaderManager, ShapeMeshes, camera, stb_image, linmath
│   └─ shaders/                  <- vertexShader.glsl, fragmentShader.glsl
└─ Textures/                     <- Carpet, Walnut, Fire, Wicker (.jpg)
```

## One-time machine setup (do this once, ever)

1. **Install vcpkg** (skip if you already have it):
   Open *Developer PowerShell for VS 2022* and run:
   ```
   cd C:\
   git clone https://github.com/microsoft/vcpkg.git
   cd vcpkg
   .\bootstrap-vcpkg.bat
   ```

2. **Integrate vcpkg with Visual Studio** (this is what makes auto-install work):
   ```
   .\vcpkg integrate install
   ```
   You'll see "Applied user-wide integration". That's it — every VS project on
   this machine can now auto-pull vcpkg packages.

## Building the project

1. Double-click **CS330Project.sln** to open in VS 2022.
2. Set the configuration bar at the top to **Debug** and **x64**
   (x64 only — Win32/x86 is not configured).
3. Press **Ctrl+Shift+B** (Build) or **F5** (Build + Run).

The **first** build will pause for a few minutes while vcpkg downloads and
compiles glew, glfw3, and glm. This is normal and only happens once — later
builds are fast. You should see a window titled
"7-1 FinalProject and Milestones" with your 3D scene.

## How asset paths work now
- Shaders load from `Utilities/shaders/...`
- Textures load from `Textures/...`
- Both are relative to the **project directory**, and the project is set to run
  the debugger from there (LocalDebuggerWorkingDirectory = $(ProjectDir)).
  So F5 from inside VS "just works." If you ever run the built .exe directly
  from x64\Debug, copy the Textures and Utilities folders next to the .exe, or
  run it from the project folder.

## Controls (standard CS-330 camera)
- W/A/S/D = move, Q/E = up/down
- Mouse = look around
- Scroll = movement speed
- P = toggle perspective/orthographic (if implemented in your ViewManager)

## Troubleshooting
- **"Cannot open include file 'GL/glew.h' / 'GLFW/glfw3.h' / 'glm/glm.hpp'"**
  → vcpkg integration didn't run. Re-run `.\vcpkg integrate install` and reopen VS.
- **Builds but window closes instantly / black screen**
  → asset path issue. Confirm you're running with F5 (not the raw .exe), and that
    the Textures and Utilities\shaders folders are present.
- **Linker errors about glew/glfw**
  → Make sure config is x64 (not Win32) and that vcpkg.json is in the project root.
- **vcpkg didn't auto-install**
  → Confirm the project shows `<VcpkgEnableManifest>true</VcpkgEnableManifest>`
    (it does) and that vcpkg integrate install was run from your vcpkg folder.

## For your capstone upgrades
Adding a new library later is one line — add it to `vcpkg.json` dependencies
(e.g. `"assimp"` for model loading) and rebuild. vcpkg fetches it automatically.
