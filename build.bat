@echo off
setlocal enabledelayedexpansion

pushd "%~dp0"

rem Configure and build the Win32 Direct3D 9 demo through CMake.
rem The project targets x86 because it links the DirectX SDK (June 2010) x86 libraries.

set "CLEAN_ONLY=0"
set "CLEAN_FIRST=0"

if /i "%~1"=="clean" set "CLEAN_ONLY=1"
if /i "%~1"=="--clean" set "CLEAN_FIRST=1"
if /i "%~1"=="/clean" set "CLEAN_FIRST=1"

if "%CLEAN_ONLY%"=="1" (
    call :CleanBuildOutput
    popd
    endlocal
    exit /b 0
)

if "%CLEAN_FIRST%"=="1" (
    call :CleanBuildOutput
)

set "DXSDK_DIR=C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)"
set "DX_INC=%DXSDK_DIR%\Include"
set "DX_LIB=%DXSDK_DIR%\Lib\x86"

if not exist "%DX_INC%\d3d9.h" (
    echo DirectX SDK include path was not found:
    echo   "%DX_INC%"
    popd
    exit /b 1
)

if not exist "%DX_LIB%\d3d9.lib" (
    echo DirectX SDK x86 library path was not found:
    echo   "%DX_LIB%"
    popd
    exit /b 1
)

if not exist assets\textures\default_diffuse.jpg (
    echo Missing required texture:
    echo   assets\textures\default_diffuse.jpg
    echo Add a diffuse texture there, then run build.bat again.
    popd
    exit /b 1
)

if not exist assets\textures\wood019_color_1k.png (
    echo Missing required texture:
    echo   assets\textures\wood019_color_1k.png
    echo Add the wood texture there, then run build.bat again.
    popd
    exit /b 1
)

if not exist assets\static_objects\wooden_cube.fbx (
    echo Missing required model:
    echo   assets\static_objects\wooden_cube.fbx
    echo Add the cube model there, then run build.bat again.
    popd
    exit /b 1
)

set "VCTOOLSBAT="
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" if exist "%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe" set "VSWHERE=%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe"
if exist "%VSWHERE%" (
    for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -find VC\Auxiliary\Build\vcvarsall.bat 2^>nul`) do (
        if not defined VCTOOLSBAT set "VCTOOLSBAT=%%i"
    )
)

if not defined VCTOOLSBAT if exist "C:\Program Files\Microsoft Visual Studio\2026\Professional\VC\Auxiliary\Build\vcvarsall.bat" (
    set "VCTOOLSBAT=C:\Program Files\Microsoft Visual Studio\2026\Professional\VC\Auxiliary\Build\vcvarsall.bat"
)
if not defined VCTOOLSBAT if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" (
    set "VCTOOLSBAT=%ProgramFiles(x86)%\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"
)
if not defined VCTOOLSBAT if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvarsall.bat" (
    set "VCTOOLSBAT=%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvarsall.bat"
)

if defined VCTOOLSBAT (
    echo Calling "%VCTOOLSBAT%" x86...
    call "%VCTOOLSBAT%" x86
)

where cl >nul 2>nul
if errorlevel 1 (
    echo cl.exe was not found.
    echo Install the Visual C++ build tools, then run build.bat again.
    popd
    exit /b 1
)

where nmake >nul 2>nul
if errorlevel 1 (
    echo nmake.exe was not found.
    echo Install the Visual C++ build tools, then run build.bat again.
    popd
    exit /b 1
)

set "CMAKE_EXE="
where cmake >nul 2>nul
if not errorlevel 1 (
    set "CMAKE_EXE=cmake"
)

if not defined CMAKE_EXE (
    if exist "%VSWHERE%" (
        for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.CMake.Project -find Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe 2^>nul`) do (
            if not defined CMAKE_EXE set "CMAKE_EXE=%%i"
        )
    )
)

if not defined CMAKE_EXE if exist "C:\Program Files\Microsoft Visual Studio\2026\Professional\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" (
    set "CMAKE_EXE=C:\Program Files\Microsoft Visual Studio\2026\Professional\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
)
if not defined CMAKE_EXE if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" (
    set "CMAKE_EXE=%ProgramFiles(x86)%\Microsoft Visual Studio\2019\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
)
if not defined CMAKE_EXE if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Professional\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" (
    set "CMAKE_EXE=%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Professional\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
)

if not defined CMAKE_EXE (
    echo cmake.exe was not found.
    echo Install CMake or the Visual Studio CMake tools, then run build.bat again.
    popd
    exit /b 1
)

echo Using CMake: "%CMAKE_EXE%"

"%CMAKE_EXE%" -S . -B build\cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DSIGMA_OUTPUT_DIR="%CD%\build"
if errorlevel 1 (
    echo CMake configure failed.
    popd
    exit /b 1
)

"%CMAKE_EXE%" --build build\cmake
if errorlevel 1 (
    echo Build failed.
    popd
    exit /b 1
)

"%CMAKE_EXE%" -E copy_directory assets build\assets
if errorlevel 1 (
    echo Asset copy failed.
    popd
    exit /b 1
)

echo.
echo Build succeeded: build\Sigma.exe
echo Runtime assets copied to: build\assets
echo Run: run.bat

popd
endlocal
exit /b 0

:CleanBuildOutput
echo Cleaning generated build output...
if exist build (
    rmdir /s /q build
)
echo Clean complete.
exit /b 0
