# Repository Guidelines

## Project Structure & Module Organization

This repository is a small Win32 Direct3D 9 C++17 demo engine. Source lives under `src/`.

- `src/main.cpp` is the Win32 entry point and loads the game scene.
- `src/Engine/` contains reusable engine code: application loop, Direct3D renderer, input, scenes, game objects, components, meshes, textures, and transforms.
- `src/Game/` contains game-specific scene composition, currently `WoodCubeScene`.
- `assets/` contains runtime content. The required texture is `assets/textures/wood.jpg`; `build.bat` copies it into `build/assets`.
- `build/` is generated output and should not be treated as source.

There is no dedicated `tests/` directory.

## Build, Test, and Development Commands

Build from a Visual Studio x86 Native Tools Command Prompt, or let the script initialize MSVC:

```bat
build.bat
```

This compiles `src/**/*.cpp`, links Direct3D 9 dependencies, writes `build\Sigma.exe`, and copies assets.

Run locally:

```bat
build\Sigma.exe
```

There is no automated test command yet. Validate changes by building and manually checking launch, texture loading, selection, keyboard rotation, mouse drag rotation, Space auto-rotation, and Esc quit.

## Coding Style & Naming Conventions

Use C++17 and match the existing style:

- 4-space indentation.
- Allman-style braces for functions, classes, namespaces, and control blocks.
- `PascalCase` for classes, methods, and free functions such as `GameObject`, `CreateObject`, and `BuildExecutableRelativePath`.
- `camelCase` for local variables and parameters.
- Private members end with `_`, for example `renderer_` and `deviceReady_`.
- Keep engine code inside `namespace Engine` and game-specific code inside `namespace Game`.

No formatter or linter is configured; keep edits focused and preserve surrounding style.

## Testing Guidelines

Automated tests are not configured. If adding tests later, place them in `tests/` and document the command here. For rendering or input changes, include manual verification notes in the pull request.

## Commit & Pull Request Guidelines

Recent history uses short descriptive messages, for example `Initial commit - Sigma c++ game Engine.` and `Merge remote README with local project`. Prefer concise messages such as `Add mesh selection component`.

Pull requests should include:

- A short description and affected modules.
- Build result for `build.bat`.
- Manual verification steps for gameplay, rendering, or input changes.
- Screenshots or short clips when visual output changes.

## Security & Configuration Tips

Do not commit local SDK paths beyond documented build assumptions. Keep generated binaries, object files, logs, and copied assets out of source review.
