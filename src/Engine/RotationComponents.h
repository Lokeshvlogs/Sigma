#pragma once

#include "Component.h"

namespace Engine
{
    class AutoRotateComponent final : public Component
    {
    public:
        AutoRotateComponent(float yawSpeed, float pitchSpeed);

        void Update(GameObject& owner, float deltaSeconds, GameContext& context) override;

    private:
        bool enabled_ = true;
        float yawSpeed_ = 0.0f;
        float pitchSpeed_ = 0.0f;
    };

    class KeyboardRotationComponent final : public Component
    {
    public:
        explicit KeyboardRotationComponent(float speed);

        void Update(GameObject& owner, float deltaSeconds, GameContext& context) override;

    private:
        float speed_ = 0.0f;
    };

    class MouseDragRotationComponent final : public Component
    {
    public:
        explicit MouseDragRotationComponent(float sensitivity);

        void Update(GameObject& owner, float deltaSeconds, GameContext& context) override;

    private:
        float sensitivity_ = 0.0f;
    };
}
