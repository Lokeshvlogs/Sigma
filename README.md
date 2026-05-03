# C++ Sigma 3D Game Engine - Win32 DirectX

Place runtime content under `assets`. The default cube texture lives at `assets/textures/wood.jpg`; `build.bat` copies the full asset tree to `build/assets` so the executable can load it at runtime.

Build from a Visual Studio x86 Native Tools Command Prompt:

```bat
build.bat
```

Run:

```bat
build\Sigma.exe
```

Project layout:

- `src/main.cpp` - thin Win32 entry point.
- `src/Engine/` - reusable engine modules: application loop, input, scene, game objects, components, Direct3D renderer, meshes, and textures.
- `src/Game/` - game-specific scenes and composition.
- `assets/textures/` - diffuse/albedo textures such as `wood.jpg`.
- `assets/bump_maps/` - normal or bump maps for future materials.
- `assets/images/` - UI, reference, splash, or non-material images.
- `assets/shaders/` - shader files for future programmable-pipeline experiments.
- `assets/static_objects/` - static meshes or exported simulation/game objects.
- `build/` - generated executable, object files, and copied runtime assets.

Controls:

- `Space` toggles automatic rotation for the selected object.
- Arrow keys rotate the selected object.
- Left-click the cube to select it.
- Left-drag the selected cube to rotate it with the mouse.
- `Esc` quits.

The engine uses a component-based object model. A game object is assembled from reusable components such as `TransformComponent`, `MeshRendererComponent`, `SelectionComponent`, `AutoRotateComponent`, `KeyboardRotationComponent`, and `MouseDragRotationComponent`.
