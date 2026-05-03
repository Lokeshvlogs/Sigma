#include "GameObject.h"

#include "GameContext.h"

#include <utility>

namespace Engine
{
    GameObject::GameObject(std::string name)
        : name_(std::move(name))
    {
    }

    void GameObject::Start()
    {
        for (auto& component : components_)
        {
            component->Start(*this);
        }
    }

    void GameObject::Update(float deltaSeconds, GameContext& context)
    {
        if (!active_)
        {
            return;
        }

        for (auto& component : components_)
        {
            component->Update(*this, deltaSeconds, context);
        }
    }

    void GameObject::Render(GameContext& context)
    {
        if (!active_)
        {
            return;
        }

        for (auto& component : components_)
        {
            component->Render(*this, context);
        }
    }
}
