#pragma once

#include "Component.h"

namespace Engine
{
    struct SceneObject;

    class SelectionComponent final : public Component
    {
    public:
        SelectionComponent(float localBoundingRadius, SceneObject* targetSceneObject);

        bool IsSelected() const { return selected_; }
        int HoveredFaceIndex() const { return hoveredFaceIndex_; }
        void Update(GameObject& owner, float deltaSeconds, GameContext& context) override;

    private:
        bool HitTest(GameObject& owner, GameContext& context) const;
        int HitTestFace(GameObject& owner, GameContext& context) const;

        float localBoundingRadius_ = 1.0f;
        SceneObject* targetSceneObject_ = nullptr;
        int hoveredFaceIndex_ = -1;
        bool selected_ = true;
    };
}
