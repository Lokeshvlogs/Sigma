#include "RotationComponents.h"

#include "GameContext.h"
#include "GameObject.h"
#include "InputManager.h"
#include "SelectionComponent.h"
#include "TransformComponent.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace Engine
{
    namespace
    {
        bool IsSelected(GameObject& owner)
        {
            auto* selection = owner.GetComponent<SelectionComponent>();
            return !selection || selection->IsSelected();
        }
    }

    AutoRotateComponent::AutoRotateComponent(float yawSpeed, float pitchSpeed)
        : yawSpeed_(yawSpeed)
        , pitchSpeed_(pitchSpeed)
    {
    }

    void AutoRotateComponent::Update(GameObject& owner, float deltaSeconds, GameContext& context)
    {
        if (context.input.WasKeyPressed(VK_SPACE))
        {
            enabled_ = !enabled_;
        }

        if (!enabled_ || !IsSelected(owner))
        {
            return;
        }

        if (auto* transform = owner.GetComponent<TransformComponent>())
        {
            transform->rotation.y += yawSpeed_ * deltaSeconds;
            transform->rotation.x += pitchSpeed_ * deltaSeconds;
        }
    }

    KeyboardRotationComponent::KeyboardRotationComponent(float speed)
        : speed_(speed)
    {
    }

    void KeyboardRotationComponent::Update(GameObject& owner, float deltaSeconds, GameContext& context)
    {
        if (!IsSelected(owner))
        {
            return;
        }

        auto* transform = owner.GetComponent<TransformComponent>();
        if (!transform)
        {
            return;
        }

        const float amount = speed_ * deltaSeconds;
        if (context.input.IsKeyDown(VK_LEFT))  { transform->rotation.y -= amount; }
        if (context.input.IsKeyDown(VK_RIGHT)) { transform->rotation.y += amount; }
        if (context.input.IsKeyDown(VK_UP))    { transform->rotation.x -= amount; }
        if (context.input.IsKeyDown(VK_DOWN))  { transform->rotation.x += amount; }
    }

    MouseDragRotationComponent::MouseDragRotationComponent(float sensitivity)
        : sensitivity_(sensitivity)
    {
    }

    void MouseDragRotationComponent::Update(GameObject& owner, float, GameContext& context)
    {
        if (!IsSelected(owner) || !context.input.IsMouseDown(MouseButton::Left))
        {
            return;
        }

        auto* transform = owner.GetComponent<TransformComponent>();
        if (!transform)
        {
            return;
        }

        transform->rotation.y += static_cast<float>(context.input.MouseDeltaX()) * sensitivity_;
        transform->rotation.x += static_cast<float>(context.input.MouseDeltaY()) * sensitivity_;
    }
}
