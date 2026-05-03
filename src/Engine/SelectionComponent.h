#pragma once

#include "Component.h"

namespace Engine
{
    class SelectionComponent final : public Component
    {
    public:
        explicit SelectionComponent(float localBoundingRadius);

        bool IsSelected() const { return selected_; }
        void Update(GameObject& owner, float deltaSeconds, GameContext& context) override;

    private:
        bool HitTest(GameObject& owner, GameContext& context) const;

        float localBoundingRadius_ = 1.0f;
        bool selected_ = true;
    };
}
