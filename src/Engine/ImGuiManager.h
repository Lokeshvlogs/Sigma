#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d9.h>

#include "imgui.h"
#include "Material.h"
#include "SceneSettings.h"

#include <string>
#include <vector>

namespace Engine
{
    class Direct3DRenderer;
    class Scene;

    class ImGuiManager
    {
    public:
        bool Initialize(HWND hwnd, IDirect3DDevice9* device);
        void Shutdown();

        bool HandleMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
        void BeginFrame();
        void EndFrame();
        void Render(Scene& scene, Direct3DRenderer& renderer, HWND hwnd);

        void InvalidateDeviceObjects();
        void CreateDeviceObjects();

        bool WantsKeyboardCapture() const { return wantsKeyboardCapture_; }
        bool WantsMouseCapture() const { return wantsMouseCapture_; }

    private:
        struct ObjectSnapshot
        {
            bool active = true;
            D3DXVECTOR3 position = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
            D3DXVECTOR3 rotation = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
            D3DXVECTOR3 scale = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
            Material material;
        };

        struct SceneSnapshot
        {
            D3DXVECTOR3 cameraEye = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
            D3DXVECTOR3 cameraTarget = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
            D3DXVECTOR3 cameraUp = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
            SceneLightingSettings lighting;
            std::vector<ObjectSnapshot> objects;
        };

        SceneSnapshot CaptureSnapshot(Scene& scene, Direct3DRenderer& renderer) const;
        void RestoreSnapshot(Scene& scene, Direct3DRenderer& renderer, const SceneSnapshot& snapshot);
        bool SnapshotsEqual(const SceneSnapshot& left, const SceneSnapshot& right) const;
        void BeginUndoEdit(Scene& scene, Direct3DRenderer& renderer);
        void CommitUndoEdit(Scene& scene, Direct3DRenderer& renderer);
        void PushUndoSnapshot(const SceneSnapshot& snapshot);
        void Undo(Scene& scene, Direct3DRenderer& renderer);
        void Redo(Scene& scene, Direct3DRenderer& renderer);

        bool initialized_ = false;
        bool frameActive_ = false;
        bool wantsKeyboardCapture_ = false;
        bool wantsMouseCapture_ = false;
        std::string saveStatus_;
        std::string selectionSceneName_;
        std::vector<bool> transformSelectionStates_;
        std::vector<SceneSnapshot> undoStack_;
        std::vector<SceneSnapshot> redoStack_;
        SceneSnapshot pendingEditSnapshot_;
        bool hasPendingEditSnapshot_ = false;
        int inspectorSelection_ = 0;
        int focusedSceneObjectIndex_ = -1;
        int gizmoOperation_ = 0;
        float zoomStep_ = 25.0f;
        float orbitStepDegrees_ = 12.0f;
        float panStep_ = 10.0f;
        ImFont* symbolFont_ = nullptr;
    };
}
