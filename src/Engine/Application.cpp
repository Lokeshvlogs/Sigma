#include "Application.h"

#include "GameContext.h"

#include <cstdio>

namespace Engine
{
    namespace
    {
        const int kClientWidth = 800;
        const int kClientHeight = 600;
        const char* kWindowClassName = "SigmaD3D9GameWindow";
    }

    Application::Application(HINSTANCE instance, int showCommand)
        : instance_(instance)
        , showCommand_(showCommand)
    {
    }

    int Application::Run(std::unique_ptr<Scene> scene)
    {
        if (!CreateMainWindow())
        {
            ShowFatalError("Could not create the Win32 window.");
            return 1;
        }

        HRESULT hr = renderer_.Initialize(hwnd_, kClientWidth, kClientHeight);
        if (FAILED(hr))
        {
            ShowFatalError("Could not initialize Direct3D 9.", hr);
            return 1;
        }

        GameContext context { input_, renderer_ };
        if (!scene->Load(context))
        {
            ShowFatalError("Could not load the game scene.");
            return 1;
        }
        scene->Start();
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

            context.requestQuit = false;
            if (input_.WasKeyPressed(VK_ESCAPE))
            {
                context.requestQuit = true;
            }

            scene->Update(deltaSeconds, context);
            if (context.requestQuit)
            {
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

            if (renderer_.PrepareFrame())
            {
                if (renderer_.BeginFrame(D3DCOLOR_XRGB(32, 36, 42)))
                {
                    renderManager_.Render(context);
                    renderer_.EndFrame();
                }
            }
        }

        scene->Unload();
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

        switch (message)
        {
        case WM_SIZE:
            if (renderer_.Device() && wParam != SIZE_MINIMIZED)
            {
                renderer_.Reset();
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

        RECT desired = { 0, 0, kClientWidth, kClientHeight };
        AdjustWindowRect(&desired, WS_OVERLAPPEDWINDOW, FALSE);

        hwnd_ = CreateWindowExA(
            0,
            kWindowClassName,
            "Sigma Engine - Wood Cube",
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
        sprintf_s(title, sizeof(title), "Sigma Engine - Wood Cube - %.1f FPS", fps);
        SetWindowTextA(hwnd_, title);
    }
}
