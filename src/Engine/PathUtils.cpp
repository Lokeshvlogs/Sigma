#include "PathUtils.h"

#include <cstdio>
#include <cstring>

namespace Engine
{
    void BuildExecutableRelativePath(const char* fileName, char* outPath, DWORD outPathSize)
    {
        DWORD length = GetModuleFileNameA(nullptr, outPath, outPathSize);
        if (length == 0 || length >= outPathSize)
        {
            sprintf_s(outPath, outPathSize, "%s", fileName);
            return;
        }

        for (DWORD i = length; i > 0; --i)
        {
            if (outPath[i - 1] == '\\' || outPath[i - 1] == '/')
            {
                outPath[i] = '\0';
                break;
            }
        }

        strncat_s(outPath, outPathSize, fileName, _TRUNCATE);
    }

    bool FileExists(const char* path)
    {
        DWORD attributes = GetFileAttributesA(path);
        return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
    }
}
