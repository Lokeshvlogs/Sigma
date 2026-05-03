#pragma once

namespace Engine
{
    class GameObject;
    struct GameContext;

    class Component
    {
    public:
        virtual ~Component() = default;

        virtual void Start(GameObject&) {}
        virtual void Update(GameObject&, float, GameContext&) {}
        virtual void Render(GameObject&, GameContext&) {}
    };
}
