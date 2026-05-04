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
- The on-screen Dear ImGui transform panel lets you pick a scene object from a dropdown and adjust translation, rotation, scale, eye, target, and up-vector values at runtime.
- Each scene also has an XML settings file under `src/Game/<SceneName>/<SceneName>.xml`; scenes are now data-driven from that XML, including window/camera defaults, scene objects, asset paths, material settings, interaction settings, and render-pass definitions.
- The panel's `Save to XML` button persists the current window size, camera, scene-object transforms, and runtime-updated material values back into that file for the next launch.
- `HeartScene` also exposes `Red Channel ID`, `Highlight Color`, and `Hover-driven highlight` controls in the ImGui panel, and uses a second `HighlightHeart.fx` pass to overlay matching inner-heart regions encoded in the mesh vertex-color red channel.

`HeartScene` uses the same keyboard and mouse rotation controls and renders the inside and outside heart meshes with a shared `Heart.fx` shader.

The engine uses a component-based object model. A game object is assembled from reusable components such as `TransformComponent`, `SelectionComponent`, `AutoRotateComponent`, `KeyboardRotationComponent`, and `MouseDragRotationComponent`. Scenes own `SceneObject` render entries with material data and ordered render-pass definitions loaded from XML; `RenderManager` renders those passes using shader assets from `assets/shaders`.
