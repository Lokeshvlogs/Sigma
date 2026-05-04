#include "SceneSettingsIO.h"

#include "PathUtils.h"

#include <tinyxml2.h>

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
                if (!name)
                {
                    continue;
                }

                ObjectTransformSettings objectSettings;
                objectSettings.name = name;
                QueryVectorAttributes(objectElement->FirstChildElement("Translation"), objectSettings.translation);
                QueryVectorAttributes(objectElement->FirstChildElement("Rotation"), objectSettings.rotation);
                QueryVectorAttributes(objectElement->FirstChildElement("Scale"), objectSettings.scale);
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

        for (const ObjectTransformSettings& object : settings.objects)
        {
            tinyxml2::XMLElement* objectElement = document.NewElement("Object");
            objectElement->SetAttribute("name", object.name.c_str());
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
        }

        return document.SaveFile(fullPath) == tinyxml2::XML_SUCCESS;
    }
}
