#include "Application.h"

#include "GameContext.h"

#include <cstdio>

namespace Engine
{
    namespace
    {
        const char* kWindowClassName = "SigmaD3D9GameWindow";
    }

    Application::Application(HINSTANCE instance, int showCommand)
        : instance_(instance)
        , showCommand_(showCommand)
    {
    }

    int Application::Run(std::unique_ptr<Scene> scene)
    {
        activeScene_ = scene.get();
        scene->LoadSettingsFromFile();
        clientWidth_ = scene->Settings().windowWidth > 0 ? scene->Settings().windowWidth : 1920;
        clientHeight_ = scene->Settings().windowHeight > 0 ? scene->Settings().windowHeight : 1080;

        if (!CreateMainWindow())
        {
            ShowFatalError("Could not create the Win32 window.");
            return 1;
        }

        HRESULT hr = renderer_.Initialize(hwnd_, clientWidth_, clientHeight_);
        if (FAILED(hr))
        {
            ShowFatalError("Could not initialize Direct3D 9.", hr);
            return 1;
        }

        if (!imgui_.Initialize(hwnd_, renderer_.Device()))
        {
            ShowFatalError("Could not initialize Dear ImGui.");
            return 1;
        }

        GameContext context { input_, renderer_ };
        if (!scene->Load(context))
        {
            ShowFatalError("Could not load the game scene.");
            imgui_.Shutdown();
            return 1;
        }

        scene->Start();
        scene->ApplyLoadedSettings(renderer_);
        renderManager_.ClearScenes();
        renderManager_.AddScene(*scene);

        QueryPerformanceFrequency(&qpcFrequency_);
        QueryPerformanceCounter(&lastCounter_);

        MSG message = {};
        while (message.message != WM_QUIT)
        {
            input_.BeginFrame();
            while (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE))
            {
                if (message.message == WM_QUIT)
                {
                    break;
                }

                TranslateMessage(&message);
                DispatchMessage(&message);
            }

            if (message.message == WM_QUIT)
            {
                break;
            }

            LARGE_INTEGER now = {};
            QueryPerformanceCounter(&now);
            float deltaSeconds = static_cast<float>(
                static_cast<double>(now.QuadPart - lastCounter_.QuadPart) /
                static_cast<double>(qpcFrequency_.QuadPart));
            lastCounter_ = now;

            if (deltaSeconds > 0.1f)
            {
                deltaSeconds = 0.1f;
            }

            imgui_.BeginFrame();
            context.uiWantsKeyboard = imgui_.WantsKeyboardCapture();
            context.uiWantsMouse = imgui_.WantsMouseCapture();

            context.requestQuit = false;
            if (!context.uiWantsKeyboard && input_.WasKeyPressed(VK_ESCAPE))
            {
                context.requestQuit = true;
            }

            scene->Update(deltaSeconds, context);
            if (context.requestQuit)
            {
                imgui_.EndFrame();
                PostQuitMessage(0);
                continue;
            }

            fpsAccumulator_ += deltaSeconds;
            ++fpsFrames_;
            if (fpsAccumulator_ >= 0.5f)
            {
                UpdateWindowTitle(static_cast<float>(fpsFrames_) / fpsAccumulator_);
                fpsAccumulator_ = 0.0f;
                fpsFrames_ = 0;
            }

            HRESULT deviceState = renderer_.DeviceCooperativeLevel();
            if (deviceState == D3DERR_DEVICELOST)
            {
                imgui_.EndFrame();
                Sleep(20);
                continue;
            }

            if (deviceState == D3DERR_DEVICENOTRESET)
            {
                imgui_.InvalidateDeviceObjects();
                if (FAILED(renderer_.Reset()))
                {
                    imgui_.EndFrame();
                    continue;
                }
                imgui_.CreateDeviceObjects();
            }

            if (renderer_.BeginFrame(D3DCOLOR_XRGB(32, 36, 42)))
            {
                renderManager_.Render(context);
                imgui_.Render(*scene, renderer_, hwnd_);
                renderer_.EndFrame();
            }
            else
            {
                imgui_.EndFrame();
            }
        }

        activeScene_ = nullptr;
        scene->Unload();
        imgui_.Shutdown();
        return static_cast<int>(message.wParam);
    }

    LRESULT CALLBACK Application::StaticWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        auto* application = reinterpret_cast<Application*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        if (message == WM_NCCREATE)
        {
            auto* createStruct = reinterpret_cast<CREATESTRUCT*>(lParam);
            application = reinterpret_cast<Application*>(createStruct->lpCreateParams);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(application));
        }

        if (application)
        {
            return application->WindowProc(hwnd, message, wParam, lParam);
        }

        return DefWindowProc(hwnd, message, wParam, lParam);
    }

    LRESULT Application::WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        input_.HandleMessage(message, wParam, lParam);
        if (imgui_.HandleMessage(hwnd, message, wParam, lParam))
        {
            return 0;
        }

        switch (message)
        {
        case WM_SIZE:
            if (renderer_.Device() && wParam != SIZE_MINIMIZED)
            {
                imgui_.InvalidateDeviceObjects();
                if (SUCCEEDED(renderer_.Reset()))
                {
                    imgui_.CreateDeviceObjects();
                }
            }
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }

        return DefWindowProc(hwnd, message, wParam, lParam);
    }

    bool Application::CreateMainWindow()
    {
        WNDCLASSEXA wc = {};
        wc.cbSize = sizeof(wc);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = StaticWindowProc;
        wc.hInstance = instance_;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
        wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
        wc.lpszClassName = kWindowClassName;

        if (!RegisterClassExA(&wc))
        {
            return false;
        }

        RECT desired = { 0, 0, clientWidth_, clientHeight_ };
        AdjustWindowRect(&desired, WS_OVERLAPPEDWINDOW, FALSE);

        hwnd_ = CreateWindowExA(
            0,
            kWindowClassName,
            "Sigma Engine - ImGui Controls",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            desired.right - desired.left,
            desired.bottom - desired.top,
            nullptr,
            nullptr,
            instance_,
            this);

        if (!hwnd_)
        {
            return false;
        }

        ShowWindow(hwnd_, showCommand_);
        UpdateWindow(hwnd_);
        return true;
    }

    void Application::ShowFatalError(const char* message, HRESULT hr)
    {
        char buffer[512];
        if (FAILED(hr))
        {
            sprintf_s(buffer, sizeof(buffer), "%s\n\nHRESULT: 0x%08lX", message, static_cast<unsigned long>(hr));
        }
        else
        {
            sprintf_s(buffer, sizeof(buffer), "%s", message);
        }

        MessageBoxA(hwnd_, buffer, "Sigma Engine Error", MB_ICONERROR | MB_OK);
    }

    void Application::UpdateWindowTitle(float fps)
    {
        char title[128];
        sprintf_s(title, sizeof(title), "Sigma Engine - ImGui Controls - %.1f FPS", fps);
        SetWindowTextA(hwnd_, title);
    }
}
