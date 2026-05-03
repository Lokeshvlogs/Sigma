#pragma once

#include <windows.h>

namespace Engine
{
    void BuildExecutableRelativePath(const char* fileName, char* outPath, DWORD outPathSize);
    bool FileExists(const char* path);
}
