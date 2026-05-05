#include "SceneSettingsIO.h"

#include "PathUtils.h"

#include <tinyxml2.h>
#include <cstring>

namespace Engine
{
    namespace
    {
        void SetVectorAttributes(tinyxml2::XMLElement& element, const D3DXVECTOR3& value)
        {
            element.SetAttribute("x", value.x);
            element.SetAttribute("y", value.y);
            element.SetAttribute("z", value.z);
        }

        bool QueryVectorAttributes(const tinyxml2::XMLElement* element, D3DXVECTOR3& value)
        {
            if (!element)
            {
                return false;
            }

            return element->QueryFloatAttribute("x", &value.x) == tinyxml2::XML_SUCCESS &&
                element->QueryFloatAttribute("y", &value.y) == tinyxml2::XML_SUCCESS &&
                element->QueryFloatAttribute("z", &value.z) == tinyxml2::XML_SUCCESS;
        }

        void SetColorAttributes(tinyxml2::XMLElement& element, D3DCOLOR color)
        {
            element.SetAttribute("r", static_cast<int>((color >> 16) & 0xff));
            element.SetAttribute("g", static_cast<int>((color >> 8) & 0xff));
            element.SetAttribute("b", static_cast<int>(color & 0xff));
            element.SetAttribute("a", static_cast<int>((color >> 24) & 0xff));
        }

        bool QueryColorAttributes(const tinyxml2::XMLElement* element, D3DCOLOR& color)
        {
            if (!element)
            {
                return false;
            }

            int red = 255;
            int green = 255;
            int blue = 255;
            int alpha = 255;
            return element->QueryIntAttribute("r", &red) == tinyxml2::XML_SUCCESS &&
                element->QueryIntAttribute("g", &green) == tinyxml2::XML_SUCCESS &&
                element->QueryIntAttribute("b", &blue) == tinyxml2::XML_SUCCESS &&
                element->QueryIntAttribute("a", &alpha) == tinyxml2::XML_SUCCESS &&
                (color = D3DCOLOR_ARGB(alpha, red, green, blue), true);
        }

        void SetFloat4Attributes(tinyxml2::XMLElement& element, const std::array<float, 4>& values)
        {
            element.SetAttribute("x", values[0]);
            element.SetAttribute("y", values[1]);
            element.SetAttribute("z", values[2]);
            element.SetAttribute("w", values[3]);
        }

        bool QueryFloat4Attributes(const tinyxml2::XMLElement* element, std::array<float, 4>& values)
        {
            if (!element)
            {
                return false;
            }

            return element->QueryFloatAttribute("x", &values[0]) == tinyxml2::XML_SUCCESS &&
                element->QueryFloatAttribute("y", &values[1]) == tinyxml2::XML_SUCCESS &&
                element->QueryFloatAttribute("z", &values[2]) == tinyxml2::XML_SUCCESS &&
                element->QueryFloatAttribute("w", &values[3]) == tinyxml2::XML_SUCCESS;
        }

        const char* RenderPassPhaseName(RenderPassPhase phase)
        {
            switch (phase)
            {
            case RenderPassPhase::Overlay:
                return "overlay";
            case RenderPassPhase::Base:
            default:
                return "base";
            }
        }

        RenderPassPhase ParseRenderPassPhase(const char* value)
        {
            if (value && strcmp(value, "overlay") == 0)
            {
                return RenderPassPhase::Overlay;
            }

            return RenderPassPhase::Base;
        }

        const char* RenderPassDrawModeName(RenderPassDrawMode drawMode)
        {
            switch (drawMode)
            {
            case RenderPassDrawMode::FaceTints:
                return "face-tints";
            case RenderPassDrawMode::HoveredFace:
                return "hovered-face";
            case RenderPassDrawMode::All:
            default:
                return "all";
            }
        }

        RenderPassDrawMode ParseRenderPassDrawMode(const char* value)
        {
            if (value && strcmp(value, "face-tints") == 0)
            {
                return RenderPassDrawMode::FaceTints;
            }

            if (value && strcmp(value, "hovered-face") == 0)
            {
                return RenderPassDrawMode::HoveredFace;
            }

            return RenderPassDrawMode::All;
        }

        const char* PixelShaderConstantSourceName(PixelShaderConstantSource source)
        {
            switch (source)
            {
            case PixelShaderConstantSource::TintColor:
                return "tint-color";
            case PixelShaderConstantSource::ShaderParameters:
                return "shader-parameters";
            case PixelShaderConstantSource::HighlightColor:
                return "highlight-color";
            case PixelShaderConstantSource::OverlayColor:
                return "overlay-color";
            case PixelShaderConstantSource::OverlayParameters:
                return "overlay-parameters";
            case PixelShaderConstantSource::None:
            default:
                return "none";
            }
        }

        PixelShaderConstantSource ParsePixelShaderConstantSource(const char* value)
        {
            if (!value || strcmp(value, "none") == 0)
            {
                return PixelShaderConstantSource::None;
            }

            if (strcmp(value, "tint-color") == 0)
            {
                return PixelShaderConstantSource::TintColor;
            }

            if (strcmp(value, "shader-parameters") == 0)
            {
                return PixelShaderConstantSource::ShaderParameters;
            }

            if (strcmp(value, "highlight-color") == 0)
            {
                return PixelShaderConstantSource::HighlightColor;
            }

            if (strcmp(value, "overlay-color") == 0)
            {
                return PixelShaderConstantSource::OverlayColor;
            }

            if (strcmp(value, "overlay-parameters") == 0)
            {
                return PixelShaderConstantSource::OverlayParameters;
            }

            return PixelShaderConstantSource::None;
        }

        const char* CullModeName(D3DCULL cullMode)
        {
            switch (cullMode)
            {
            case D3DCULL_CW:
                return "cw";
            case D3DCULL_CCW:
                return "ccw";
            case D3DCULL_NONE:
            default:
                return "none";
            }
        }

        D3DCULL ParseCullMode(const char* value)
        {
            if (value && strcmp(value, "cw") == 0)
            {
                return D3DCULL_CW;
            }

            if (value && strcmp(value, "ccw") == 0)
            {
                return D3DCULL_CCW;
            }

            return D3DCULL_NONE;
        }

        const char* BlendName(D3DBLEND blend)
        {
            switch (blend)
            {
            case D3DBLEND_ZERO:
                return "zero";
            case D3DBLEND_ONE:
                return "one";
            case D3DBLEND_SRCALPHA:
                return "src-alpha";
            case D3DBLEND_INVSRCALPHA:
                return "inv-src-alpha";
            default:
                return "one";
            }
        }

        D3DBLEND ParseBlend(const char* value)
        {
            if (!value || strcmp(value, "one") == 0)
            {
                return D3DBLEND_ONE;
            }

            if (strcmp(value, "zero") == 0)
            {
                return D3DBLEND_ZERO;
            }

            if (strcmp(value, "src-alpha") == 0)
            {
                return D3DBLEND_SRCALPHA;
            }

            if (strcmp(value, "inv-src-alpha") == 0)
            {
                return D3DBLEND_INVSRCALPHA;
            }

            return D3DBLEND_ONE;
        }
    }

    bool LoadSceneSettingsXml(const char* relativePath, SceneSettings& outSettings)
    {
        char fullPath[MAX_PATH];
        BuildProjectRelativePath(relativePath, fullPath, MAX_PATH);

        tinyxml2::XMLDocument document;
        if (document.LoadFile(fullPath) != tinyxml2::XML_SUCCESS)
        {
            return false;
        }

        const tinyxml2::XMLElement* root = document.FirstChildElement("SceneSettings");
        if (!root)
        {
            return false;
        }

        if (const char* sceneName = root->Attribute("name"))
        {
            outSettings.sceneName = sceneName;
        }

        const tinyxml2::XMLElement* windowElement = root->FirstChildElement("Window");
        if (windowElement)
        {
            windowElement->QueryIntAttribute("width", &outSettings.windowWidth);
            windowElement->QueryIntAttribute("height", &outSettings.windowHeight);
        }

        const tinyxml2::XMLElement* cameraElement = root->FirstChildElement("Camera");
        if (cameraElement)
        {
            QueryVectorAttributes(cameraElement->FirstChildElement("Eye"), outSettings.cameraEye);
            QueryVectorAttributes(cameraElement->FirstChildElement("Target"), outSettings.cameraTarget);
            QueryVectorAttributes(cameraElement->FirstChildElement("Up"), outSettings.cameraUp);
        }

        outSettings.objects.clear();
        const tinyxml2::XMLElement* objectsElement = root->FirstChildElement("Objects");
        if (objectsElement)
        {
            for (const tinyxml2::XMLElement* objectElement = objectsElement->FirstChildElement("Object");
                objectElement;
                objectElement = objectElement->NextSiblingElement("Object"))
            {
                const char* name = objectElement->Attribute("name");
                const char* meshPath = objectElement->Attribute("mesh");
                if (!name)
                {
                    continue;
                }

                SceneObjectSettings objectSettings;
                objectSettings.name = name;
                if (meshPath)
                {
                    objectSettings.meshPath = meshPath;
                }

                QueryVectorAttributes(objectElement->FirstChildElement("Translation"), objectSettings.translation);
                QueryVectorAttributes(objectElement->FirstChildElement("Rotation"), objectSettings.rotation);
                QueryVectorAttributes(objectElement->FirstChildElement("Scale"), objectSettings.scale);

                const tinyxml2::XMLElement* materialElement = objectElement->FirstChildElement("Material");
                if (materialElement)
                {
                    if (const char* diffusePath = materialElement->Attribute("diffuseTexture"))
                    {
                        objectSettings.material.diffuseTexturePath = diffusePath;
                    }

                    if (const char* diffuseFallbackPath = materialElement->Attribute("diffuseFallbackTexture"))
                    {
                        objectSettings.material.diffuseFallbackTexturePath = diffuseFallbackPath;
                    }

                    if (const char* normalPath = materialElement->Attribute("normalMap"))
                    {
                        objectSettings.material.normalMapPath = normalPath;
                    }

                    if (const char* bumpPath = materialElement->Attribute("bumpMap"))
                    {
                        objectSettings.material.bumpMapPath = bumpPath;
                    }

                    QueryColorAttributes(materialElement->FirstChildElement("DefaultTint"), objectSettings.material.defaultTint);
                    QueryColorAttributes(materialElement->FirstChildElement("SelectedTint"), objectSettings.material.selectedTint);
                    QueryFloat4Attributes(materialElement->FirstChildElement("ShaderParameters"), objectSettings.material.shaderParameters);
                    QueryFloat4Attributes(materialElement->FirstChildElement("HighlightColor"), objectSettings.material.highlightColor);
                    QueryFloat4Attributes(materialElement->FirstChildElement("OverlayColor"), objectSettings.material.overlayColor);
                    QueryFloat4Attributes(materialElement->FirstChildElement("OverlayParameters"), objectSettings.material.overlayParameters);

                    objectSettings.material.faceTints.clear();
                    const tinyxml2::XMLElement* faceTintsElement = materialElement->FirstChildElement("FaceTints");
                    if (faceTintsElement)
                    {
                        for (const tinyxml2::XMLElement* tintElement = faceTintsElement->FirstChildElement("Tint");
                            tintElement;
                            tintElement = tintElement->NextSiblingElement("Tint"))
                        {
                            D3DCOLOR tint = D3DCOLOR_ARGB(255, 255, 255, 255);
                            if (QueryColorAttributes(tintElement, tint))
                            {
                                objectSettings.material.faceTints.push_back(tint);
                            }
                        }
                    }
                }

                const tinyxml2::XMLElement* interactionElement = objectElement->FirstChildElement("Interaction");
                if (interactionElement)
                {
                    interactionElement->QueryBoolAttribute("selectionEnabled", &objectSettings.interaction.selectionEnabled);
                    interactionElement->QueryFloatAttribute("selectionBoundingRadius", &objectSettings.interaction.selectionBoundingRadius);
                    interactionElement->QueryFloatAttribute("keyboardRotationSpeed", &objectSettings.interaction.keyboardRotationSpeed);
                    interactionElement->QueryFloatAttribute("mouseDragSensitivity", &objectSettings.interaction.mouseDragSensitivity);
                }

                objectSettings.renderPasses.clear();
                const tinyxml2::XMLElement* renderPassesElement = objectElement->FirstChildElement("RenderPasses");
                if (renderPassesElement)
                {
                    for (const tinyxml2::XMLElement* renderPassElement = renderPassesElement->FirstChildElement("RenderPass");
                        renderPassElement;
                        renderPassElement = renderPassElement->NextSiblingElement("RenderPass"))
                    {
                        RenderPassSettings renderPass;
                        renderPass.phase = ParseRenderPassPhase(renderPassElement->Attribute("phase"));
                        renderPass.drawMode = ParseRenderPassDrawMode(renderPassElement->Attribute("drawMode"));
                        renderPass.constant0Source = ParsePixelShaderConstantSource(renderPassElement->Attribute("constant0"));
                        renderPass.constant1Source = ParsePixelShaderConstantSource(renderPassElement->Attribute("constant1"));
                        renderPass.renderStates.cullMode = ParseCullMode(renderPassElement->Attribute("cullMode"));
                        renderPass.renderStates.sourceBlend = ParseBlend(renderPassElement->Attribute("sourceBlend"));
                        renderPass.renderStates.destinationBlend = ParseBlend(renderPassElement->Attribute("destinationBlend"));

                        if (const char* shaderProgramPath = renderPassElement->Attribute("shaderProgram"))
                        {
                            renderPass.shaderProgramPath = shaderProgramPath;
                        }
                        else if (const char* legacyShaderPath = renderPassElement->Attribute("shader"))
                        {
                            renderPass.shaderProgramPath = legacyShaderPath;
                        }

                        if (const char* techniqueName = renderPassElement->Attribute("technique"))
                        {
                            renderPass.techniqueName = techniqueName;
                        }

                        if (const char* passName = renderPassElement->Attribute("pass"))
                        {
                            renderPass.passName = passName;
                        }

                        renderPassElement->QueryBoolAttribute("zEnabled", &renderPass.renderStates.zEnabled);
                        renderPassElement->QueryBoolAttribute("zWriteEnabled", &renderPass.renderStates.zWriteEnabled);
                        renderPassElement->QueryBoolAttribute("alphaBlendEnabled", &renderPass.renderStates.alphaBlendEnabled);
                        renderPassElement->QueryBoolAttribute("bindMaterialTextures", &renderPass.bindMaterialTextures);
                        renderPassElement->QueryBoolAttribute("enabled", &renderPass.enabled);
                        objectSettings.renderPasses.push_back(renderPass);
                    }
                }

                outSettings.objects.push_back(objectSettings);
            }
        }

        return true;
    }

    bool SaveSceneSettingsXml(const char* relativePath, const SceneSettings& settings)
    {
        char fullPath[MAX_PATH];
        BuildProjectRelativePath(relativePath, fullPath, MAX_PATH);

        tinyxml2::XMLDocument document;
        document.InsertEndChild(document.NewDeclaration());

        tinyxml2::XMLElement* root = document.NewElement("SceneSettings");
        root->SetAttribute("name", settings.sceneName.c_str());
        document.InsertEndChild(root);

        tinyxml2::XMLElement* windowElement = document.NewElement("Window");
        windowElement->SetAttribute("width", settings.windowWidth);
        windowElement->SetAttribute("height", settings.windowHeight);
        root->InsertEndChild(windowElement);

        tinyxml2::XMLElement* cameraElement = document.NewElement("Camera");
        root->InsertEndChild(cameraElement);

        tinyxml2::XMLElement* eyeElement = document.NewElement("Eye");
        SetVectorAttributes(*eyeElement, settings.cameraEye);
        cameraElement->InsertEndChild(eyeElement);

        tinyxml2::XMLElement* targetElement = document.NewElement("Target");
        SetVectorAttributes(*targetElement, settings.cameraTarget);
        cameraElement->InsertEndChild(targetElement);

        tinyxml2::XMLElement* upElement = document.NewElement("Up");
        SetVectorAttributes(*upElement, settings.cameraUp);
        cameraElement->InsertEndChild(upElement);

        tinyxml2::XMLElement* objectsElement = document.NewElement("Objects");
        root->InsertEndChild(objectsElement);

        for (const SceneObjectSettings& object : settings.objects)
        {
            tinyxml2::XMLElement* objectElement = document.NewElement("Object");
            objectElement->SetAttribute("name", object.name.c_str());
            objectElement->SetAttribute("mesh", object.meshPath.c_str());
            objectsElement->InsertEndChild(objectElement);

            tinyxml2::XMLElement* translationElement = document.NewElement("Translation");
            SetVectorAttributes(*translationElement, object.translation);
            objectElement->InsertEndChild(translationElement);

            tinyxml2::XMLElement* rotationElement = document.NewElement("Rotation");
            SetVectorAttributes(*rotationElement, object.rotation);
            objectElement->InsertEndChild(rotationElement);

            tinyxml2::XMLElement* scaleElement = document.NewElement("Scale");
            SetVectorAttributes(*scaleElement, object.scale);
            objectElement->InsertEndChild(scaleElement);

            tinyxml2::XMLElement* materialElement = document.NewElement("Material");
            materialElement->SetAttribute("diffuseTexture", object.material.diffuseTexturePath.c_str());
            materialElement->SetAttribute("diffuseFallbackTexture", object.material.diffuseFallbackTexturePath.c_str());
            materialElement->SetAttribute("normalMap", object.material.normalMapPath.c_str());
            materialElement->SetAttribute("bumpMap", object.material.bumpMapPath.c_str());
            objectElement->InsertEndChild(materialElement);

            tinyxml2::XMLElement* defaultTintElement = document.NewElement("DefaultTint");
            SetColorAttributes(*defaultTintElement, object.material.defaultTint);
            materialElement->InsertEndChild(defaultTintElement);

            tinyxml2::XMLElement* selectedTintElement = document.NewElement("SelectedTint");
            SetColorAttributes(*selectedTintElement, object.material.selectedTint);
            materialElement->InsertEndChild(selectedTintElement);

            tinyxml2::XMLElement* shaderParametersElement = document.NewElement("ShaderParameters");
            SetFloat4Attributes(*shaderParametersElement, object.material.shaderParameters);
            materialElement->InsertEndChild(shaderParametersElement);

            tinyxml2::XMLElement* highlightColorElement = document.NewElement("HighlightColor");
            SetFloat4Attributes(*highlightColorElement, object.material.highlightColor);
            materialElement->InsertEndChild(highlightColorElement);

            tinyxml2::XMLElement* overlayColorElement = document.NewElement("OverlayColor");
            SetFloat4Attributes(*overlayColorElement, object.material.overlayColor);
            materialElement->InsertEndChild(overlayColorElement);

            tinyxml2::XMLElement* overlayParametersElement = document.NewElement("OverlayParameters");
            SetFloat4Attributes(*overlayParametersElement, object.material.overlayParameters);
            materialElement->InsertEndChild(overlayParametersElement);

            tinyxml2::XMLElement* faceTintsElement = document.NewElement("FaceTints");
            materialElement->InsertEndChild(faceTintsElement);
            for (D3DCOLOR tint : object.material.faceTints)
            {
                tinyxml2::XMLElement* tintElement = document.NewElement("Tint");
                SetColorAttributes(*tintElement, tint);
                faceTintsElement->InsertEndChild(tintElement);
            }

            tinyxml2::XMLElement* interactionElement = document.NewElement("Interaction");
            interactionElement->SetAttribute("selectionEnabled", object.interaction.selectionEnabled);
            interactionElement->SetAttribute("selectionBoundingRadius", object.interaction.selectionBoundingRadius);
            interactionElement->SetAttribute("keyboardRotationSpeed", object.interaction.keyboardRotationSpeed);
            interactionElement->SetAttribute("mouseDragSensitivity", object.interaction.mouseDragSensitivity);
            objectElement->InsertEndChild(interactionElement);

            tinyxml2::XMLElement* renderPassesElement = document.NewElement("RenderPasses");
            objectElement->InsertEndChild(renderPassesElement);
            for (const RenderPassSettings& renderPass : object.renderPasses)
            {
                tinyxml2::XMLElement* renderPassElement = document.NewElement("RenderPass");
                renderPassElement->SetAttribute("phase", RenderPassPhaseName(renderPass.phase));
                renderPassElement->SetAttribute("shaderProgram", renderPass.shaderProgramPath.c_str());
                renderPassElement->SetAttribute("technique", renderPass.techniqueName.c_str());
                renderPassElement->SetAttribute("pass", renderPass.passName.c_str());
                renderPassElement->SetAttribute("zEnabled", renderPass.renderStates.zEnabled);
                renderPassElement->SetAttribute("zWriteEnabled", renderPass.renderStates.zWriteEnabled);
                renderPassElement->SetAttribute("cullMode", CullModeName(renderPass.renderStates.cullMode));
                renderPassElement->SetAttribute("alphaBlendEnabled", renderPass.renderStates.alphaBlendEnabled);
                renderPassElement->SetAttribute("sourceBlend", BlendName(renderPass.renderStates.sourceBlend));
                renderPassElement->SetAttribute("destinationBlend", BlendName(renderPass.renderStates.destinationBlend));
                renderPassElement->SetAttribute("bindMaterialTextures", renderPass.bindMaterialTextures);
                renderPassElement->SetAttribute("drawMode", RenderPassDrawModeName(renderPass.drawMode));
                renderPassElement->SetAttribute("constant0", PixelShaderConstantSourceName(renderPass.constant0Source));
                renderPassElement->SetAttribute("constant1", PixelShaderConstantSourceName(renderPass.constant1Source));
                renderPassElement->SetAttribute("enabled", renderPass.enabled);
                renderPassesElement->InsertEndChild(renderPassElement);
            }
        }

        return document.SaveFile(fullPath) == tinyxml2::XML_SUCCESS;
    }
}
