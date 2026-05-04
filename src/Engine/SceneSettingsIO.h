#pragma once

#include "SceneSettings.h"

namespace Engine
{
    bool LoadSceneSettingsXml(const char* relativePath, SceneSettings& outSettings);
    bool SaveSceneSettingsXml(const char* relativePath, const SceneSettings& settings);
}
