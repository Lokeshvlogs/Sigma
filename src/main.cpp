#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "Engine/Application.h"
#include "Game/HeartScene.h"
#include "Game/SampleScene.h"

#include <algorithm>
#include <cctype>
#include <memory>
#include <string>

namespace
{
    std::unique_ptr<Engine::Scene> CreateSceneFromCommandLine(LPSTR commandLine)
    {
        std::string sceneName = commandLine ? commandLine : "";
        std::transform(
            sceneName.begin(),
            sceneName.end(),
            sceneName.begin(),
            [](unsigned char value)
            {
                return static_cast<char>(std::tolower(value));
            });

        if (sceneName.find("heart") != std::string::npos)
        {
            return std::make_unique<Game::HeartScene>();
        }

        return std::make_unique<Game::SampleScene>();
    }
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR commandLine, int showCommand)
{
    Engine::Application application(instance, showCommand);
    return application.Run(CreateSceneFromCommandLine(commandLine));
}
