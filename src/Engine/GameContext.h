#pragma once

namespace Engine
{
    class Direct3DRenderer;
    class InputManager;

    struct GameContext
    {
        InputManager& input;
        Direct3DRenderer& renderer;
        bool requestQuit = false;
    };
}
