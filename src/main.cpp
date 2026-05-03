#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "Engine/Application.h"
#include "Game/WoodCubeScene.h"

#include <memory>

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int showCommand)
{
    Engine::Application application(instance, showCommand);
    return application.Run(std::make_unique<Game::WoodCubeScene>());
}
