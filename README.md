# C++ Sigma 3D Game Engine - Win32 DirectX

Place runtime content under `assets`. The sample diffuse texture lives at `assets/textures/default_diffuse.jpg`; the CMake build copies the full asset tree to `build/assets` so the executable can load it at runtime.

Build:

```bat
build.bat
```

Run:

```bat
run.bat
```

Launch the heart scene:

```bat
run.bat heart
```

Project layout:

- `src/main.cpp` - thin Win32 entry point.
- `src/Engine/` - reusable engine modules: application loop, input, scene, game objects, components, Direct3D renderer, meshes, and textures.
- `src/Game/` - game-specific scenes and composition, including `SampleScene` and `HeartScene`.
- `assets/textures/` - diffuse/albedo textures such as `default_diffuse.jpg`.
- `assets/bump_maps/` - normal or bump maps for future materials.
- `assets/images/` - UI, reference, splash, or non-material images.
- `assets/shaders/` - HLSL pixel shaders used by material and hover-highlight render passes.
- `assets/static_objects/` - static meshes or exported simulation/game objects.
- `build/` - generated executable, CMake files, object files, and copied runtime assets.

Controls:

- `Space` toggles automatic rotation for the selected object.
- Arrow keys rotate the selected object.
- Left-click the cube to select it.
- Hover a cube face to highlight it with a blue shader pass.
- Left-drag the selected cube to rotate it with the mouse.
- `Esc` quits.

`HeartScene` uses the same keyboard and mouse rotation controls and renders the inside and outside heart meshes with a shared `Heart.fx` shader.

The engine uses a component-based object model. A game object is assembled from reusable components such as `TransformComponent`, `SelectionComponent`, `AutoRotateComponent`, `KeyboardRotationComponent`, and `MouseDragRotationComponent`. Scenes own `SceneObject` render entries with material data and render states; `RenderManager` renders registered scenes using shader passes loaded from `assets/shaders`.
