#pragma once

#include "Component.h"

#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace Engine
{
    class GameObject
    {
    public:
        explicit GameObject(std::string name);

        const std::string& Name() const { return name_; }
        bool IsActive() const { return active_; }
        void SetActive(bool active) { active_ = active; }

        template <typename T, typename... Args>
        T& AddComponent(Args&&... args)
        {
            static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
            auto component = std::make_unique<T>(std::forward<Args>(args)...);
            T& reference = *component;
            components_.push_back(std::move(component));
            return reference;
        }

        template <typename T>
        T* GetComponent()
        {
            static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
            for (auto& component : components_)
            {
                if (auto typed = dynamic_cast<T*>(component.get()))
                {
                    return typed;
                }
            }
            return nullptr;
        }

        void Start();
        void Update(float deltaSeconds, GameContext& context);
        void Render(GameContext& context);

    private:
        std::string name_;
        bool active_ = true;
        std::vector<std::unique_ptr<Component>> components_;
    };
}
