#include "ImGuiManager.h"

#include "Direct3DRenderer.h"
#include "Scene.h"
#include "SceneObject.h"
#include "TransformComponent.h"

#include "imgui.h"
#include "backends/imgui_impl_dx9.h"
#include "backends/imgui_impl_win32.h"

#include <algorithm>
#include <cmath>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

namespace Engine
{
    namespace
    {
        constexpr float kMinimumScale = 0.01f;
        constexpr float kMinimumCameraDistance = 5.0f;

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

        const char* LightingTypeLabel(SceneLightingType type)
        {
            switch (type)
            {
            case SceneLightingType::Studio:
                return "Studio";
            case SceneLightingType::Dramatic:
                return "Dramatic";
            case SceneLightingType::Balanced:
            default:
                return "Balanced";
            }
        }

        void ApplyLightingPreset(SceneLightingSettings& lighting, SceneLightingType type)
        {
            lighting.type = type;
            switch (type)
            {
            case SceneLightingType::Studio:
                lighting.keyDirection = D3DXVECTOR3(-0.20f, 0.85f, 0.45f);
                lighting.fillDirection = D3DXVECTOR3(0.65f, 0.12f, -0.25f);
                lighting.keyIntensity = 1.20f;
                lighting.fillIntensity = 0.55f;
                lighting.ambientIntensity = 0.30f;
                lighting.warmth = 0.0f;
                break;

            case SceneLightingType::Dramatic:
                lighting.keyDirection = D3DXVECTOR3(-0.58f, 0.65f, 0.42f);
                lighting.fillDirection = D3DXVECTOR3(0.40f, 0.08f, -0.72f);
                lighting.keyIntensity = 1.30f;
                lighting.fillIntensity = 0.18f;
                lighting.ambientIntensity = 0.16f;
                lighting.warmth = 0.25f;
                break;

            case SceneLightingType::Balanced:
            default:
                lighting.keyDirection = D3DXVECTOR3(-0.35f, 0.72f, 0.60f);
                lighting.fillDirection = D3DXVECTOR3(0.45f, 0.22f, -0.40f);
                lighting.keyIntensity = 1.00f;
                lighting.fillIntensity = 0.36f;
                lighting.ambientIntensity = 0.24f;
                lighting.warmth = 0.10f;
                break;
            }
        }

        void ScaleLightingIntensity(SceneLightingSettings& lighting, float scale)
        {
            lighting.keyIntensity = std::clamp(lighting.keyIntensity * scale, 0.0f, 3.0f);
            lighting.fillIntensity = std::clamp(lighting.fillIntensity * scale, 0.0f, 2.0f);
            lighting.ambientIntensity = std::clamp(lighting.ambientIntensity * scale, 0.0f, 1.5f);
        }

        bool IconButton(ImFont* symbolFont, const char* id, const char* icon, const char* tooltip, const ImVec2& size)
        {
            if (symbolFont)
            {
                ImGui::PushFont(symbolFont);
            }

            const std::string label = std::string(icon) + "##" + id;
            const bool pressed = ImGui::Button(label.c_str(), size);

            if (symbolFont)
            {
                ImGui::PopFont();
            }

            if (tooltip && ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("%s", tooltip);
            }

            return pressed;
        }

        void SyncSelectionState(
            const Scene& scene,
            std::string& selectionSceneName,
            std::vector<bool>& selectionStates,
            std::string& saveStatus)
        {
            if (selectionSceneName != scene.SceneName())
            {
                selectionSceneName = scene.SceneName();
                selectionStates.clear();
                saveStatus.clear();
            }

            selectionStates.resize(scene.SceneObjects().size(), false);
        }

        std::vector<TransformComponent*> SelectedTransforms(
            std::vector<std::unique_ptr<SceneObject>>& sceneObjects,
            const std::vector<bool>& selectionStates)
        {
            std::vector<TransformComponent*> transforms;
            for (size_t index = 0; index < sceneObjects.size() && index < selectionStates.size(); ++index)
            {
                if (!selectionStates[index])
                {
                    continue;
                }

                if (TransformComponent* transform = ObjectTransform(sceneObjects[index].get()))
                {
                    transforms.push_back(transform);
                }
            }

            return transforms;
        }

        D3DXVECTOR3 AverageVector3(const std::vector<TransformComponent*>& transforms, D3DXVECTOR3 TransformComponent::* member)
        {
            if (transforms.empty())
            {
                return D3DXVECTOR3(0.0f, 0.0f, 0.0f);
            }

            D3DXVECTOR3 average(0.0f, 0.0f, 0.0f);
            for (TransformComponent* transform : transforms)
            {
                average += transform->*member;
            }

            const float inverseCount = 1.0f / static_cast<float>(transforms.size());
            average *= inverseCount;
            return average;
        }

        void ApplyTransformDelta(
            const std::vector<TransformComponent*>& transforms,
            const D3DXVECTOR3& translationDelta,
            const D3DXVECTOR3& rotationDelta,
            const D3DXVECTOR3& scaleDelta)
        {
            for (TransformComponent* transform : transforms)
            {
                transform->position += translationDelta;
                transform->rotation += rotationDelta;
                transform->scale += scaleDelta;
                transform->scale.x = std::max(transform->scale.x, kMinimumScale);
                transform->scale.y = std::max(transform->scale.y, kMinimumScale);
                transform->scale.z = std::max(transform->scale.z, kMinimumScale);
            }
        }

        D3DXVECTOR3 CameraOffset(const Direct3DRenderer& renderer)
        {
            return renderer.Eye() - renderer.Target();
        }

        float CameraDistance(const Direct3DRenderer& renderer)
        {
            const D3DXVECTOR3 offset = CameraOffset(renderer);
            return std::max(kMinimumCameraDistance, D3DXVec3Length(&offset));
        }

        D3DXVECTOR3 CameraForward(const Direct3DRenderer& renderer)
        {
            D3DXVECTOR3 forward = renderer.Target() - renderer.Eye();
            if (D3DXVec3LengthSq(&forward) < 0.0001f)
            {
                return D3DXVECTOR3(0.0f, 0.0f, 1.0f);
            }

            D3DXVec3Normalize(&forward, &forward);
            return forward;
        }

        D3DXVECTOR3 CameraRight(const Direct3DRenderer& renderer)
        {
            D3DXVECTOR3 forward = CameraForward(renderer);
            D3DXVECTOR3 right;
            D3DXVec3Cross(&right, &renderer.Up(), &forward);
            if (D3DXVec3LengthSq(&right) < 0.0001f)
            {
                return D3DXVECTOR3(1.0f, 0.0f, 0.0f);
            }

            D3DXVec3Normalize(&right, &right);
            return right;
        }

        D3DXVECTOR3 CameraUp(const Direct3DRenderer& renderer)
        {
            D3DXVECTOR3 forward = CameraForward(renderer);
            D3DXVECTOR3 right = CameraRight(renderer);
            D3DXVECTOR3 up;
            D3DXVec3Cross(&up, &forward, &right);
            if (D3DXVec3LengthSq(&up) < 0.0001f)
            {
                return D3DXVECTOR3(0.0f, 1.0f, 0.0f);
            }

            D3DXVec3Normalize(&up, &up);
            return up;
        }

        void SetCameraDistance(Direct3DRenderer& renderer, float distance)
        {
            D3DXVECTOR3 offset = CameraOffset(renderer);
            if (D3DXVec3LengthSq(&offset) < 0.0001f)
            {
                offset = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
            }

            D3DXVec3Normalize(&offset, &offset);
            renderer.SetCameraTransform(
                renderer.Target() + offset * std::max(distance, kMinimumCameraDistance),
                renderer.Target(),
                renderer.Up());
        }

        void ApplyCameraZoom(Direct3DRenderer& renderer, float delta)
        {
            SetCameraDistance(renderer, CameraDistance(renderer) + delta);
        }

        void ApplyCameraOrbit(Direct3DRenderer& renderer, float yawDeltaDegrees, float pitchDeltaDegrees)
        {
            D3DXVECTOR3 offset = CameraOffset(renderer);
            float distance = D3DXVec3Length(&offset);
            if (distance < kMinimumCameraDistance)
            {
                distance = kMinimumCameraDistance;
                offset = D3DXVECTOR3(0.0f, 0.0f, distance);
            }

            float yaw = atan2f(offset.x, offset.z);
            float pitch = asinf(std::clamp(offset.y / distance, -1.0f, 1.0f));
            yaw += D3DXToRadian(yawDeltaDegrees);
            pitch = std::clamp(pitch + D3DXToRadian(pitchDeltaDegrees), D3DXToRadian(-85.0f), D3DXToRadian(85.0f));

            const float cosPitch = cosf(pitch);
            D3DXVECTOR3 newOffset(
                sinf(yaw) * cosPitch * distance,
                sinf(pitch) * distance,
                cosf(yaw) * cosPitch * distance);

            renderer.SetCameraTransform(
                renderer.Target() + newOffset,
                renderer.Target(),
                D3DXVECTOR3(0.0f, 1.0f, 0.0f));
        }

        void ApplyCameraPan(Direct3DRenderer& renderer, float rightDelta, float upDelta)
        {
            const D3DXVECTOR3 right = CameraRight(renderer);
            const D3DXVECTOR3 up = CameraUp(renderer);
            const D3DXVECTOR3 translation = right * rightDelta + up * upDelta;

            renderer.SetCameraTransform(
                renderer.Eye() + translation,
                renderer.Target() + translation,
                renderer.Up());
        }

        bool AlmostEqual(float left, float right)
        {
            return fabsf(left - right) < 0.0001f;
        }

        bool VectorEqual(const D3DXVECTOR3& left, const D3DXVECTOR3& right)
        {
            return AlmostEqual(left.x, right.x) && AlmostEqual(left.y, right.y) && AlmostEqual(left.z, right.z);
        }

        bool Float4Equal(const std::array<float, 4>& left, const std::array<float, 4>& right)
        {
            for (size_t index = 0; index < left.size(); ++index)
            {
                if (!AlmostEqual(left[index], right[index]))
                {
                    return false;
                }
            }

            return true;
        }

        bool MaterialEqual(const Material& left, const Material& right)
        {
            return left.diffuseTexturePath == right.diffuseTexturePath
                && left.normalMapPath == right.normalMapPath
                && left.bumpMapPath == right.bumpMapPath
                && left.faceTints == right.faceTints
                && left.defaultTint == right.defaultTint
                && left.selectedTint == right.selectedTint
                && Float4Equal(left.shaderParameters, right.shaderParameters)
                && Float4Equal(left.highlightColor, right.highlightColor)
                && Float4Equal(left.overlayColor, right.overlayColor)
                && Float4Equal(left.overlayParameters, right.overlayParameters);
        }

        bool LightingEqual(const SceneLightingSettings& left, const SceneLightingSettings& right)
        {
            return left.type == right.type
                && VectorEqual(left.keyDirection, right.keyDirection)
                && VectorEqual(left.fillDirection, right.fillDirection)
                && AlmostEqual(left.keyIntensity, right.keyIntensity)
                && AlmostEqual(left.fillIntensity, right.fillIntensity)
                && AlmostEqual(left.ambientIntensity, right.ambientIntensity)
                && AlmostEqual(left.warmth, right.warmth);
        }

        void DrawSelectedObjectGizmos(
            const Direct3DRenderer& renderer,
            std::vector<std::unique_ptr<SceneObject>>& sceneObjects,
            const std::vector<bool>& selectionStates,
            int gizmoOperation)
        {
            ImDrawList* drawList = ImGui::GetBackgroundDrawList();
            D3DVIEWPORT9 viewport = renderer.Viewport();
            D3DXMATRIX identity;
            D3DXMatrixIdentity(&identity);

            const ImU32 moveColor = IM_COL32(80, 220, 120, 230);
            const ImU32 rotateColor = IM_COL32(255, 205, 70, 230);
            const ImU32 scaleColor = IM_COL32(90, 170, 255, 230);
            const ImU32 color = gizmoOperation == 1 ? rotateColor : gizmoOperation == 2 ? scaleColor : moveColor;

            for (size_t index = 0; index < sceneObjects.size() && index < selectionStates.size(); ++index)
            {
                if (!selectionStates[index])
                {
                    continue;
                }

                TransformComponent* transform = ObjectTransform(sceneObjects[index].get());
                if (!transform)
                {
                    continue;
                }

                D3DXVECTOR3 screenPosition;
                D3DXVec3Project(
                    &screenPosition,
                    &transform->position,
                    &viewport,
                    &renderer.ProjectionMatrix(),
                    &renderer.ViewMatrix(),
                    &identity);

                if (screenPosition.z < 0.0f || screenPosition.z > 1.0f)
                {
                    continue;
                }

                const ImVec2 center(screenPosition.x, screenPosition.y);
                drawList->AddCircle(center, 12.0f, color, 28, 2.0f);
                drawList->AddLine(center, ImVec2(center.x + 38.0f, center.y), IM_COL32(255, 80, 80, 230), 2.0f);
                drawList->AddLine(center, ImVec2(center.x, center.y - 38.0f), IM_COL32(80, 220, 120, 230), 2.0f);
                drawList->AddLine(center, ImVec2(center.x + 24.0f, center.y + 24.0f), IM_COL32(90, 170, 255, 230), 2.0f);
            }
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
        if (ImFont* uiFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 18.0f))
        {
            io.FontDefault = uiFont;
        }

        symbolFont_ = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\seguisym.ttf", 20.0f);

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
        symbolFont_ = nullptr;
        saveStatus_.clear();
        selectionSceneName_.clear();
        transformSelectionStates_.clear();
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

    ImGuiManager::SceneSnapshot ImGuiManager::CaptureSnapshot(Scene& scene, Direct3DRenderer& renderer) const
    {
        SceneSnapshot snapshot;
        snapshot.cameraEye = renderer.Eye();
        snapshot.cameraTarget = renderer.Target();
        snapshot.cameraUp = renderer.Up();
        snapshot.lighting = scene.Settings().lighting;

        const auto& sceneObjects = scene.SceneObjects();
        snapshot.objects.reserve(sceneObjects.size());
        for (const auto& sceneObject : sceneObjects)
        {
            ObjectSnapshot objectSnapshot;
            if (sceneObject && sceneObject->gameObject)
            {
                objectSnapshot.active = sceneObject->gameObject->IsActive();
                if (TransformComponent* transform = ObjectTransform(sceneObject.get()))
                {
                    objectSnapshot.position = transform->position;
                    objectSnapshot.rotation = transform->rotation;
                    objectSnapshot.scale = transform->scale;
                }

                objectSnapshot.material = sceneObject->material;
            }

            snapshot.objects.push_back(objectSnapshot);
        }

        return snapshot;
    }

    void ImGuiManager::RestoreSnapshot(Scene& scene, Direct3DRenderer& renderer, const SceneSnapshot& snapshot)
    {
        renderer.SetCameraTransform(snapshot.cameraEye, snapshot.cameraTarget, snapshot.cameraUp);
        scene.MutableSettings().lighting = snapshot.lighting;

        auto& sceneObjects = scene.SceneObjects();
        const size_t objectCount = std::min(sceneObjects.size(), snapshot.objects.size());
        for (size_t index = 0; index < objectCount; ++index)
        {
            SceneObject* sceneObject = sceneObjects[index].get();
            if (!sceneObject || !sceneObject->gameObject)
            {
                continue;
            }

            const ObjectSnapshot& objectSnapshot = snapshot.objects[index];
            sceneObject->gameObject->SetActive(objectSnapshot.active);
            if (TransformComponent* transform = ObjectTransform(sceneObject))
            {
                transform->position = objectSnapshot.position;
                transform->rotation = objectSnapshot.rotation;
                transform->scale = objectSnapshot.scale;
            }

            sceneObject->material = objectSnapshot.material;
        }
    }

    bool ImGuiManager::SnapshotsEqual(const SceneSnapshot& left, const SceneSnapshot& right) const
    {
        if (!VectorEqual(left.cameraEye, right.cameraEye)
            || !VectorEqual(left.cameraTarget, right.cameraTarget)
            || !VectorEqual(left.cameraUp, right.cameraUp)
            || !LightingEqual(left.lighting, right.lighting)
            || left.objects.size() != right.objects.size())
        {
            return false;
        }

        for (size_t index = 0; index < left.objects.size(); ++index)
        {
            const ObjectSnapshot& leftObject = left.objects[index];
            const ObjectSnapshot& rightObject = right.objects[index];
            if (leftObject.active != rightObject.active
                || !VectorEqual(leftObject.position, rightObject.position)
                || !VectorEqual(leftObject.rotation, rightObject.rotation)
                || !VectorEqual(leftObject.scale, rightObject.scale)
                || !MaterialEqual(leftObject.material, rightObject.material))
            {
                return false;
            }
        }

        return true;
    }

    void ImGuiManager::PushUndoSnapshot(const SceneSnapshot& snapshot)
    {
        if (!undoStack_.empty() && SnapshotsEqual(undoStack_.back(), snapshot))
        {
            return;
        }

        undoStack_.push_back(snapshot);
        if (undoStack_.size() > 64)
        {
            undoStack_.erase(undoStack_.begin());
        }

        redoStack_.clear();
    }

    void ImGuiManager::BeginUndoEdit(Scene& scene, Direct3DRenderer& renderer)
    {
        if (!hasPendingEditSnapshot_)
        {
            pendingEditSnapshot_ = CaptureSnapshot(scene, renderer);
            hasPendingEditSnapshot_ = true;
        }
    }

    void ImGuiManager::CommitUndoEdit(Scene& scene, Direct3DRenderer& renderer)
    {
        if (!hasPendingEditSnapshot_)
        {
            return;
        }

        const SceneSnapshot currentSnapshot = CaptureSnapshot(scene, renderer);
        if (!SnapshotsEqual(pendingEditSnapshot_, currentSnapshot))
        {
            PushUndoSnapshot(pendingEditSnapshot_);
        }

        hasPendingEditSnapshot_ = false;
    }

    void ImGuiManager::Undo(Scene& scene, Direct3DRenderer& renderer)
    {
        if (undoStack_.empty())
        {
            return;
        }

        redoStack_.push_back(CaptureSnapshot(scene, renderer));
        RestoreSnapshot(scene, renderer, undoStack_.back());
        undoStack_.pop_back();
    }

    void ImGuiManager::Redo(Scene& scene, Direct3DRenderer& renderer)
    {
        if (redoStack_.empty())
        {
            return;
        }

        undoStack_.push_back(CaptureSnapshot(scene, renderer));
        RestoreSnapshot(scene, renderer, redoStack_.back());
        redoStack_.pop_back();
    }

    void ImGuiManager::Render(Scene& scene, Direct3DRenderer& renderer, HWND hwnd)
    {
        if (!initialized_ || !frameActive_)
        {
            return;
        }

        auto& sceneObjects = scene.SceneObjects();
        SyncSelectionState(scene, selectionSceneName_, transformSelectionStates_, saveStatus_);

        const ImGuiIO& imguiIo = ImGui::GetIO();
        const float leftPanelWidth = std::min(430.0f, std::max(340.0f, imguiIo.DisplaySize.x * 0.27f));
        const float rightPanelWidth = std::min(470.0f, std::max(360.0f, imguiIo.DisplaySize.x * 0.30f));
        const float panelHeight = imguiIo.DisplaySize.y;
        constexpr int kInspectorNone = 0;
        constexpr int kInspectorCamera = 1;
        constexpr int kInspectorLight = 2;
        constexpr int kInspectorObjects = 3;

        auto recordIfChanged = [&](const SceneSnapshot& before)
        {
            const SceneSnapshot after = CaptureSnapshot(scene, renderer);
            if (!SnapshotsEqual(before, after))
            {
                PushUndoSnapshot(before);
            }
        };

        auto selectObjectInspectorFallback = [&]()
        {
            focusedSceneObjectIndex_ = -1;
            for (int index = 0; index < static_cast<int>(transformSelectionStates_.size()); ++index)
            {
                if (transformSelectionStates_[index])
                {
                    focusedSceneObjectIndex_ = index;
                    inspectorSelection_ = kInspectorObjects;
                    return;
                }
            }

            if (inspectorSelection_ == kInspectorObjects)
            {
                inspectorSelection_ = kInspectorNone;
            }
        };

        DrawSelectedObjectGizmos(renderer, sceneObjects, transformSelectionStates_, gizmoOperation_);

        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(leftPanelWidth, panelHeight), ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.42f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.0f, 14.0f));
        if (ImGui::Begin("Scene Editor", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
        {
            ImGui::BeginDisabled(undoStack_.empty());
            if (IconButton(symbolFont_, "UndoScene", u8"\u21B6", "Undo scene edit", ImVec2(34.0f, 30.0f)))
            {
                Undo(scene, renderer);
            }
            ImGui::EndDisabled();
            ImGui::SameLine();
            ImGui::BeginDisabled(redoStack_.empty());
            if (IconButton(symbolFont_, "RedoScene", u8"\u21B7", "Redo scene edit", ImVec2(34.0f, 30.0f)))
            {
                Redo(scene, renderer);
            }
            ImGui::EndDisabled();

            ImGui::SeparatorText("Scene Graph");
            if (ImGui::TreeNodeEx(scene.SceneName(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth))
            {
                if (ImGui::Selectable("Camera", inspectorSelection_ == kInspectorCamera))
                {
                    inspectorSelection_ = kInspectorCamera;
                    focusedSceneObjectIndex_ = -1;
                }
                if (ImGui::Selectable("Scene Light", inspectorSelection_ == kInspectorLight))
                {
                    inspectorSelection_ = kInspectorLight;
                    focusedSceneObjectIndex_ = -1;
                }

                if (ImGui::TreeNodeEx("Scene Objects", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth))
                {
                    if (ImGui::BeginTable("SceneGraphObjects", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp))
                    {
                        ImGui::TableSetupColumn("V", ImGuiTableColumnFlags_WidthFixed, 26.0f);
                        ImGui::TableSetupColumn("S", ImGuiTableColumnFlags_WidthFixed, 26.0f);
                        ImGui::TableSetupColumn("Object");

                        for (int index = 0; index < static_cast<int>(sceneObjects.size()); ++index)
                        {
                            SceneObject* sceneObject = sceneObjects[index].get();
                            ImGui::PushID(index);
                            ImGui::TableNextRow();

                            ImGui::TableSetColumnIndex(0);
                            bool visible = sceneObject && sceneObject->gameObject && sceneObject->gameObject->IsActive();
                            if (ImGui::Checkbox("##visible", &visible) && sceneObject && sceneObject->gameObject)
                            {
                                const SceneSnapshot before = CaptureSnapshot(scene, renderer);
                                sceneObject->gameObject->SetActive(visible);
                                recordIfChanged(before);
                            }
                            if (ImGui::IsItemHovered())
                            {
                                ImGui::SetTooltip("Visible");
                            }

                            ImGui::TableSetColumnIndex(1);
                            bool selected = transformSelectionStates_[index];
                            if (ImGui::Checkbox("##selected", &selected))
                            {
                                transformSelectionStates_[index] = selected;
                                if (selected)
                                {
                                    inspectorSelection_ = kInspectorObjects;
                                    focusedSceneObjectIndex_ = index;
                                }
                                else if (focusedSceneObjectIndex_ == index)
                                {
                                    selectObjectInspectorFallback();
                                }
                            }
                            if (ImGui::IsItemHovered())
                            {
                                ImGui::SetTooltip("Selected");
                            }

                            ImGui::TableSetColumnIndex(2);
                            const bool rowSelected = transformSelectionStates_[index];
                            if (ImGui::Selectable(ObjectName(sceneObject), rowSelected))
                            {
                                if (!imguiIo.KeyCtrl)
                                {
                                    std::fill(transformSelectionStates_.begin(), transformSelectionStates_.end(), false);
                                }

                                transformSelectionStates_[index] = true;
                                inspectorSelection_ = kInspectorObjects;
                                focusedSceneObjectIndex_ = index;
                            }

                            ImGui::PopID();
                        }

                        ImGui::EndTable();
                    }
                    ImGui::TreePop();
                }

                ImGui::TreePop();
            }

            ImGui::SeparatorText("Selection");
            if (ImGui::Button("Select All"))
            {
                std::fill(transformSelectionStates_.begin(), transformSelectionStates_.end(), true);
                inspectorSelection_ = kInspectorObjects;
                selectObjectInspectorFallback();
            }
            ImGui::SameLine();
            if (ImGui::Button("Clear"))
            {
                std::fill(transformSelectionStates_.begin(), transformSelectionStates_.end(), false);
                focusedSceneObjectIndex_ = -1;
                if (inspectorSelection_ == kInspectorObjects)
                {
                    inspectorSelection_ = kInspectorNone;
                }
            }

            ImGui::SeparatorText("Scene Controls");
            const SceneSnapshot sceneControlsBefore = CaptureSnapshot(scene, renderer);
            scene.RenderImGuiControls();
            recordIfChanged(sceneControlsBefore);

            ImGui::SeparatorText("Persistence");
            if (ImGui::Button("Save XML"))
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
        ImGui::PopStyleVar(3);

        ImGui::SetNextWindowPos(ImVec2(imguiIo.DisplaySize.x - rightPanelWidth, 0.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(rightPanelWidth, panelHeight), ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.42f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.0f, 14.0f));
        if (ImGui::Begin("Scene Inspector", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
        {
            if (inspectorSelection_ == kInspectorCamera)
            {
                ImGui::SeparatorText("Camera");

                float cameraDistance = CameraDistance(renderer);
                const SceneSnapshot beforeDistance = CaptureSnapshot(scene, renderer);
                if (ImGui::SliderFloat("Zoom Distance", &cameraDistance, kMinimumCameraDistance, 1500.0f))
                {
                    SetCameraDistance(renderer, cameraDistance);
                    recordIfChanged(beforeDistance);
                }

                ImGui::SetNextItemWidth(150.0f);
                ImGui::DragFloat("Zoom Step", &zoomStep_, 1.0f, 1.0f, 250.0f);
                ImGui::SameLine();
                if (IconButton(symbolFont_, "ZoomIn", u8"\u2295", "Zoom in", ImVec2(36.0f, 30.0f)))
                {
                    const SceneSnapshot before = CaptureSnapshot(scene, renderer);
                    ApplyCameraZoom(renderer, -zoomStep_);
                    recordIfChanged(before);
                }
                ImGui::SameLine();
                if (IconButton(symbolFont_, "ZoomOut", u8"\u2296", "Zoom out", ImVec2(36.0f, 30.0f)))
                {
                    const SceneSnapshot before = CaptureSnapshot(scene, renderer);
                    ApplyCameraZoom(renderer, zoomStep_);
                    recordIfChanged(before);
                }

                ImGui::SetNextItemWidth(150.0f);
                ImGui::DragFloat("Rotate Step", &orbitStepDegrees_, 1.0f, 1.0f, 90.0f);
                ImGui::SameLine();
                if (IconButton(symbolFont_, "RotateLeft", u8"\u21BA", "Rotate left", ImVec2(36.0f, 30.0f)))
                {
                    const SceneSnapshot before = CaptureSnapshot(scene, renderer);
                    ApplyCameraOrbit(renderer, -orbitStepDegrees_, 0.0f);
                    recordIfChanged(before);
                }
                ImGui::SameLine();
                if (IconButton(symbolFont_, "RotateRight", u8"\u21BB", "Rotate right", ImVec2(36.0f, 30.0f)))
                {
                    const SceneSnapshot before = CaptureSnapshot(scene, renderer);
                    ApplyCameraOrbit(renderer, orbitStepDegrees_, 0.0f);
                    recordIfChanged(before);
                }
                ImGui::SameLine();
                if (IconButton(symbolFont_, "RotateUp", u8"\u2191", "Rotate up", ImVec2(36.0f, 30.0f)))
                {
                    const SceneSnapshot before = CaptureSnapshot(scene, renderer);
                    ApplyCameraOrbit(renderer, 0.0f, orbitStepDegrees_);
                    recordIfChanged(before);
                }
                ImGui::SameLine();
                if (IconButton(symbolFont_, "RotateDown", u8"\u2193", "Rotate down", ImVec2(36.0f, 30.0f)))
                {
                    const SceneSnapshot before = CaptureSnapshot(scene, renderer);
                    ApplyCameraOrbit(renderer, 0.0f, -orbitStepDegrees_);
                    recordIfChanged(before);
                }

                ImGui::SetNextItemWidth(150.0f);
                ImGui::DragFloat("Pan Step", &panStep_, 0.5f, 0.1f, 100.0f);
                ImGui::SameLine();
                if (IconButton(symbolFont_, "PanLeft", u8"\u2190", "Pan left", ImVec2(36.0f, 30.0f)))
                {
                    const SceneSnapshot before = CaptureSnapshot(scene, renderer);
                    ApplyCameraPan(renderer, -panStep_, 0.0f);
                    recordIfChanged(before);
                }
                ImGui::SameLine();
                if (IconButton(symbolFont_, "PanRight", u8"\u2192", "Pan right", ImVec2(36.0f, 30.0f)))
                {
                    const SceneSnapshot before = CaptureSnapshot(scene, renderer);
                    ApplyCameraPan(renderer, panStep_, 0.0f);
                    recordIfChanged(before);
                }
                ImGui::SameLine();
                if (IconButton(symbolFont_, "PanUp", u8"\u2191", "Pan up", ImVec2(36.0f, 30.0f)))
                {
                    const SceneSnapshot before = CaptureSnapshot(scene, renderer);
                    ApplyCameraPan(renderer, 0.0f, panStep_);
                    recordIfChanged(before);
                }
                ImGui::SameLine();
                if (IconButton(symbolFont_, "PanDown", u8"\u2193", "Pan down", ImVec2(36.0f, 30.0f)))
                {
                    const SceneSnapshot before = CaptureSnapshot(scene, renderer);
                    ApplyCameraPan(renderer, 0.0f, -panStep_);
                    recordIfChanged(before);
                }

                D3DXVECTOR3 eye = renderer.Eye();
                D3DXVECTOR3 target = renderer.Target();
                D3DXVECTOR3 up = renderer.Up();
                const SceneSnapshot before = CaptureSnapshot(scene, renderer);
                bool cameraChanged = false;
                cameraChanged |= ImGui::DragFloat3("Eye", &eye.x, 1.0f);
                cameraChanged |= ImGui::DragFloat3("Target", &target.x, 1.0f);
                cameraChanged |= ImGui::DragFloat3("Up", &up.x, 0.01f);
                if (cameraChanged)
                {
                    renderer.SetCameraTransform(eye, target, up);
                    recordIfChanged(before);
                }
            }
            else if (inspectorSelection_ == kInspectorLight)
            {
                ImGui::SeparatorText("Scene Light");
                SceneLightingSettings& lighting = scene.MutableSettings().lighting;
                static const char* lightingOptions[] = { "Balanced", "Studio", "Dramatic" };
                int lightingType = static_cast<int>(lighting.type);
                const SceneSnapshot beforePreset = CaptureSnapshot(scene, renderer);
                if (ImGui::Combo("Preset", &lightingType, lightingOptions, IM_ARRAYSIZE(lightingOptions)))
                {
                    ApplyLightingPreset(lighting, static_cast<SceneLightingType>(lightingType));
                    recordIfChanged(beforePreset);
                }

                if (ImGui::Button("Dim 10%"))
                {
                    const SceneSnapshot before = CaptureSnapshot(scene, renderer);
                    ScaleLightingIntensity(lighting, 0.9f);
                    recordIfChanged(before);
                }
                ImGui::SameLine();
                if (ImGui::Button("Brighten 10%"))
                {
                    const SceneSnapshot before = CaptureSnapshot(scene, renderer);
                    ScaleLightingIntensity(lighting, 1.1f);
                    recordIfChanged(before);
                }

                const SceneSnapshot beforeLighting = CaptureSnapshot(scene, renderer);
                bool lightingChanged = false;
                lightingChanged |= ImGui::SliderFloat("Key Intensity", &lighting.keyIntensity, 0.0f, 3.0f);
                lightingChanged |= ImGui::SliderFloat("Fill Intensity", &lighting.fillIntensity, 0.0f, 2.0f);
                lightingChanged |= ImGui::SliderFloat("Ambient Intensity", &lighting.ambientIntensity, 0.0f, 1.5f);
                lightingChanged |= ImGui::SliderFloat("Warmth", &lighting.warmth, -1.0f, 1.0f);
                lightingChanged |= ImGui::DragFloat3("Key Direction", &lighting.keyDirection.x, 0.01f, -1.0f, 1.0f);
                lightingChanged |= ImGui::DragFloat3("Fill Direction", &lighting.fillDirection.x, 0.01f, -1.0f, 1.0f);
                if (lightingChanged)
                {
                    recordIfChanged(beforeLighting);
                }

                ImGui::Text("Preset: %s", LightingTypeLabel(lighting.type));
            }
            else if (inspectorSelection_ == kInspectorObjects)
            {
                std::vector<TransformComponent*> selectedTransforms = SelectedTransforms(sceneObjects, transformSelectionStates_);
                if (selectedTransforms.empty())
                {
                    ImGui::TextUnformatted("No scene objects selected.");
                }
                else
                {
                    ImGui::SeparatorText("Object Inspector");
                    ImGui::Text("%d selected", static_cast<int>(selectedTransforms.size()));

                    if (ImGui::Button("Show Selected"))
                    {
                        const SceneSnapshot before = CaptureSnapshot(scene, renderer);
                        for (size_t index = 0; index < sceneObjects.size() && index < transformSelectionStates_.size(); ++index)
                        {
                            if (transformSelectionStates_[index] && sceneObjects[index] && sceneObjects[index]->gameObject)
                            {
                                sceneObjects[index]->gameObject->SetActive(true);
                            }
                        }
                        recordIfChanged(before);
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Hide Selected"))
                    {
                        const SceneSnapshot before = CaptureSnapshot(scene, renderer);
                        for (size_t index = 0; index < sceneObjects.size() && index < transformSelectionStates_.size(); ++index)
                        {
                            if (transformSelectionStates_[index] && sceneObjects[index] && sceneObjects[index]->gameObject)
                            {
                                sceneObjects[index]->gameObject->SetActive(false);
                            }
                        }
                        recordIfChanged(before);
                    }

                    ImGui::SeparatorText("Transform Gizmo");
                    ImGui::RadioButton("Move", &gizmoOperation_, 0);
                    ImGui::SameLine();
                    ImGui::RadioButton("Rotate", &gizmoOperation_, 1);
                    ImGui::SameLine();
                    ImGui::RadioButton("Scale", &gizmoOperation_, 2);

                    const D3DXVECTOR3 averageTranslation = AverageVector3(selectedTransforms, &TransformComponent::position);
                    const D3DXVECTOR3 averageRotation = AverageVector3(selectedTransforms, &TransformComponent::rotation);
                    const D3DXVECTOR3 averageScale = AverageVector3(selectedTransforms, &TransformComponent::scale);
                    D3DXVECTOR3 editedTranslation = averageTranslation;
                    D3DXVECTOR3 editedRotation = averageRotation;
                    D3DXVECTOR3 editedScale = averageScale;

                    const SceneSnapshot beforeTransform = CaptureSnapshot(scene, renderer);
                    bool transformChanged = false;
                    if (gizmoOperation_ == 0)
                    {
                        transformChanged = ImGui::DragFloat3("Move", &editedTranslation.x, 0.1f);
                        if (transformChanged)
                        {
                            ApplyTransformDelta(selectedTransforms, editedTranslation - averageTranslation, D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f));
                        }
                    }
                    else if (gizmoOperation_ == 1)
                    {
                        transformChanged = ImGui::DragFloat3("Rotate", &editedRotation.x, 0.01f);
                        if (transformChanged)
                        {
                            ApplyTransformDelta(selectedTransforms, D3DXVECTOR3(0.0f, 0.0f, 0.0f), editedRotation - averageRotation, D3DXVECTOR3(0.0f, 0.0f, 0.0f));
                        }
                    }
                    else
                    {
                        transformChanged = ImGui::DragFloat3("Scale", &editedScale.x, 0.01f, kMinimumScale, 1000.0f);
                        if (transformChanged)
                        {
                            ApplyTransformDelta(selectedTransforms, D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f), editedScale - averageScale);
                        }
                    }

                    if (transformChanged)
                    {
                        recordIfChanged(beforeTransform);
                    }

                    ImGui::SeparatorText("Selected Objects");
                    for (int index = 0; index < static_cast<int>(sceneObjects.size()); ++index)
                    {
                        if (index >= static_cast<int>(transformSelectionStates_.size()) || !transformSelectionStates_[index])
                        {
                            continue;
                        }

                        SceneObject* sceneObject = sceneObjects[index].get();
                        if (!sceneObject)
                        {
                            continue;
                        }

                        ImGui::PushID(index);
                        if (ImGui::TreeNodeEx(ObjectName(sceneObject), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth))
                        {
                            ImGui::TextWrapped("Mesh: %s", sceneObject->meshAssetPath.c_str());
                            if (sceneObject->mesh)
                            {
                                ImGui::Text("Faces: %d", sceneObject->mesh->FaceCount());
                                ImGui::Text("Bounds: %.2f", sceneObject->mesh->BoundingRadius());
                            }

                            bool visible = sceneObject->gameObject && sceneObject->gameObject->IsActive();
                            if (ImGui::Checkbox("Visible", &visible) && sceneObject->gameObject)
                            {
                                const SceneSnapshot before = CaptureSnapshot(scene, renderer);
                                sceneObject->gameObject->SetActive(visible);
                                recordIfChanged(before);
                            }

                            if (TransformComponent* transform = ObjectTransform(sceneObject))
                            {
                                const SceneSnapshot before = CaptureSnapshot(scene, renderer);
                                bool singleTransformChanged = false;
                                singleTransformChanged |= ImGui::DragFloat3("Translation", &transform->position.x, 0.1f);
                                singleTransformChanged |= ImGui::DragFloat3("Rotation", &transform->rotation.x, 0.01f);
                                singleTransformChanged |= ImGui::DragFloat3("Scale", &transform->scale.x, 0.01f, kMinimumScale, 1000.0f);
                                transform->scale.x = std::max(transform->scale.x, kMinimumScale);
                                transform->scale.y = std::max(transform->scale.y, kMinimumScale);
                                transform->scale.z = std::max(transform->scale.z, kMinimumScale);
                                if (singleTransformChanged)
                                {
                                    recordIfChanged(before);
                                }
                            }

                            ImGui::TextWrapped("Diffuse: %s", sceneObject->material.diffuseTexturePath.c_str());
                            const SceneSnapshot beforeMaterial = CaptureSnapshot(scene, renderer);
                            bool materialChanged = false;
                            materialChanged |= ImGui::ColorEdit4("Highlight", sceneObject->material.highlightColor.data());
                            materialChanged |= ImGui::ColorEdit4("Overlay", sceneObject->material.overlayColor.data());
                            materialChanged |= ImGui::DragFloat4("Shader", sceneObject->material.shaderParameters.data(), 0.01f);
                            materialChanged |= ImGui::DragFloat4("Overlay Params", sceneObject->material.overlayParameters.data(), 0.01f);
                            if (materialChanged)
                            {
                                recordIfChanged(beforeMaterial);
                            }

                            ImGui::TreePop();
                        }
                        ImGui::PopID();
                    }
                }
            }
            else
            {
                ImGui::TextUnformatted("Select Camera, Scene Light, or one or more scene objects.");
            }
        }
        ImGui::End();
        ImGui::PopStyleVar(3);

        wantsKeyboardCapture_ = imguiIo.WantCaptureKeyboard;
        wantsMouseCapture_ = imguiIo.WantCaptureMouse;

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
