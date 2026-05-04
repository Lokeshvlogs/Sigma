#include "SampleScene.h"

#include "../Engine/GameContext.h"

namespace Game
{
    bool SampleScene::Load(Engine::GameContext& context)
    {
        return LoadSceneObjectsFromSettings(context);
    }
}
