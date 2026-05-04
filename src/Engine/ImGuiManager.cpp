#include "ImGuiManager.h"

#include "Direct3DRenderer.h"
#include "Scene.h"
#include "SceneObject.h"
#include "TransformComponent.h"

#include "imgui.h"
#include "backends/imgui_impl_dx9.h"
#include "backends/imgui_impl_win32.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

namespace Engine
{
    namespace
    {
        const char* ObjectName(const SceneObject* sceneObject)
        {
            if (!sceneObject || !sceneObject->gameObject)
            {
                return "<unnamed>";
            }

            return sceneObject->gameObject->Name().c_str();
        }

        TransformComponent* ObjectTransform(SceneObject* sceneObject)
        {
            if (!sceneObject || !sceneObject->gameObject)
            {
                return nullptr;
            }

            return sceneObject->gameObject->GetComponent<TransformComponent>();
        }
    }

    bool ImGuiManager::Initialize(HWND hwnd, IDirect3DDevice9* device)
    {
        if (initialized_)
        {
            return true;
        }

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigWindowsMoveFromTitleBarOnly = true;

        if (!ImGui_ImplWin32_Init(hwnd))
        {
            ImGui::DestroyContext();
            return false;
        }

        if (!ImGui_ImplDX9_Init(device))
        {
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
            return false;
        }

        initialized_ = true;
        return true;
    }

    void ImGuiManager::Shutdown()
    {
        if (!initialized_)
        {
            return;
        }

        if (frameActive_)
        {
            EndFrame();
        }

        ImGui_ImplDX9_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        initialized_ = false;
        frameActive_ = false;
        wantsKeyboardCapture_ = false;
        wantsMouseCapture_ = false;
        selectedSceneObjectIndex_ = 0;
        saveStatus_.clear();
    }

    bool ImGuiManager::HandleMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        if (!initialized_)
        {
            return false;
        }

        return ImGui_ImplWin32_WndProcHandler(hwnd, message, wParam, lParam) != 0;
    }

    void ImGuiManager::BeginFrame()
    {
        if (!initialized_ || frameActive_)
        {
            return;
        }

        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        frameActive_ = true;

        const ImGuiIO& io = ImGui::GetIO();
        wantsKeyboardCapture_ = io.WantCaptureKeyboard;
        wantsMouseCapture_ = io.WantCaptureMouse;
    }

    void ImGuiManager::EndFrame()
    {
        if (!initialized_ || !frameActive_)
        {
            return;
        }

        ImGui::EndFrame();
        frameActive_ = false;
    }

    void ImGuiManager::Render(Scene& scene, Direct3DRenderer& renderer, HWND hwnd)
    {
        if (!initialized_ || !frameActive_)
        {
            return;
        }

        auto& sceneObjects = scene.SceneObjects();
        if (sceneObjects.empty())
        {
            selectedSceneObjectIndex_ = 0;
        }
        else if (selectedSceneObjectIndex_ >= static_cast<int>(sceneObjects.size()))
        {
            selectedSceneObjectIndex_ = static_cast<int>(sceneObjects.size()) - 1;
        }

        ImGui::SetNextWindowPos(ImVec2(16.0f, 16.0f), ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImVec2(370.0f, 0.0f), ImGuiCond_Once);

        if (ImGui::Begin("Transform Controls"))
        {
            if (sceneObjects.empty())
            {
                ImGui::TextUnformatted("No scene objects are available.");
            }
            else
            {
                const char* previewName = ObjectName(sceneObjects[selectedSceneObjectIndex_].get());
                if (ImGui::BeginCombo("Scene Object", previewName))
                {
                    for (int index = 0; index < static_cast<int>(sceneObjects.size()); ++index)
                    {
                        const bool selected = index == selectedSceneObjectIndex_;
                        if (ImGui::Selectable(ObjectName(sceneObjects[index].get()), selected))
                        {
                            selectedSceneObjectIndex_ = index;
                        }

                        if (selected)
                        {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }

                TransformComponent* transform = ObjectTransform(sceneObjects[selectedSceneObjectIndex_].get());
                if (transform)
                {
                    ImGui::SeparatorText("Object Transform");
                    ImGui::DragFloat3("Translation", &transform->position.x, 0.1f);
                    ImGui::DragFloat3("Rotation", &transform->rotation.x, 0.01f);
                    ImGui::DragFloat3("Scale", &transform->scale.x, 0.01f, 0.01f, 1000.0f);
                }
                else
                {
                    ImGui::TextUnformatted("The selected scene object has no transform component.");
                }
            }

            D3DXVECTOR3 eye = renderer.Eye();
            D3DXVECTOR3 target = renderer.Target();
            D3DXVECTOR3 up = renderer.Up();

            ImGui::SeparatorText("Camera");
            bool cameraChanged = false;
            cameraChanged |= ImGui::DragFloat3("Eye", &eye.x, 1.0f);
            cameraChanged |= ImGui::DragFloat3("Target", &target.x, 1.0f);
            cameraChanged |= ImGui::DragFloat3("Up", &up.x, 0.01f);
            if (cameraChanged)
            {
                renderer.SetCameraTransform(eye, target, up);
            }

            if (ImGui::Button("Save to XML"))
            {
                RECT client = {};
                GetClientRect(hwnd, &client);
                const int width = client.right - client.left;
                const int height = client.bottom - client.top;
                saveStatus_ = scene.SaveSettingsToFile(renderer, width, height)
                    ? std::string("Saved settings to ") + scene.SettingsFilePath()
                    : std::string("Failed to save settings to ") + scene.SettingsFilePath();
            }

            if (!saveStatus_.empty())
            {
                ImGui::TextWrapped("%s", saveStatus_.c_str());
            }
        }
        ImGui::End();

        ImGui::EndFrame();
        ImGui::Render();
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
        frameActive_ = false;
    }

    void ImGuiManager::InvalidateDeviceObjects()
    {
        if (initialized_)
        {
            ImGui_ImplDX9_InvalidateDeviceObjects();
        }
    }

    void ImGuiManager::CreateDeviceObjects()
    {
        if (initialized_)
        {
            ImGui_ImplDX9_CreateDeviceObjects();
        }
    }
}
