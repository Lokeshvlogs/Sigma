@echo off
setlocal enabledelayedexpansion

rem README
rem 1. Place runtime textures at assets\textures, especially assets\textures\wood.jpg.
rem 2. This script will attempt to initialize the Visual Studio C++ build
rem    environment (cl.exe) automatically if it's not already available.
rem    If automatic detection fails, open a Visual Studio x86 Native Tools
rem    Command Prompt and run: build.bat.
rem 3. The script copies assets into build\assets; run: build\Sigma.exe.
rem 4. This script targets x86 and uses the DirectX SDK (June 2010) paths below.

set "DXSDK_DIR=C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)"
set "DX_INC=%DXSDK_DIR%\Include"
set "DX_LIB=%DXSDK_DIR%\Lib\x86"

if not exist "%DX_INC%\d3d9.h" (
    echo DirectX SDK include path was not found:
    echo   "%DX_INC%"
    exit /b 1
)

if not exist "%DX_LIB%\d3d9.lib" (
    echo DirectX SDK x86 library path was not found:
    echo   "%DX_LIB%"
    exit /b 1
)

rem Ensure cl.exe is available; try to initialize VS environment automatically.
where cl >nul 2>nul
if errorlevel 1 (
    echo cl.exe was not found in PATH. Attempting to initialize Visual Studio environment...
    set "VCVARS_CALLED="

    rem 1) Try vswhere (if installed) to find the latest VS with VC tools
    set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
    if not exist "%VSWHERE%" if exist "%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe" set "VSWHERE=%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe"
    if exist "%VSWHERE%" (
        for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath 2^>nul`) do set "VSINSTALL=%%i"
        if defined VSINSTALL (
            set "VCTOOLSBAT=!VSINSTALL!\VC\Auxiliary\Build\vcvarsall.bat"
            if exist "!VCTOOLSBAT!" (
                echo Calling "!VCTOOLSBAT!" x86...
                call "!VCTOOLSBAT!" x86
                set "VCVARS_CALLED=1"
            )
        )
    )

    rem 2) Check the explicit Visual Studio 2026 path provided by the user
    if not defined VCVARS_CALLED (
        if exist "C:\Program Files\Microsoft Visual Studio\2026\VC\Auxiliary\Build\vcvarsall.bat" (
            set "VCTOOLSBAT=C:\Program Files\Microsoft Visual Studio\2026\VC\Auxiliary\Build\vcvarsall.bat"
            echo Calling "!VCTOOLSBAT!" x86...
            call "!VCTOOLSBAT!" x86
            set "VCVARS_CALLED=1"
        )
    )

    rem 3) Try a set of common VS install locations sequentially to avoid parsing issues
    if not defined VCVARS_CALLED if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" (
        set "VCTOOLSBAT=%ProgramFiles(x86)%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat"
        echo Calling "!VCTOOLSBAT!" x86...
        call "!VCTOOLSBAT!" x86
        set "VCVARS_CALLED=1"
    )
    if not defined VCVARS_CALLED if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat" (
        set "VCTOOLSBAT=%ProgramFiles(x86)%\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat"
        echo Calling "!VCTOOLSBAT!" x86...
        call "!VCTOOLSBAT!" x86
        set "VCVARS_CALLED=1"
    )
    if not defined VCVARS_CALLED if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" (
        set "VCTOOLSBAT=%ProgramFiles(x86)%\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat"
        echo Calling "!VCTOOLSBAT!" x86...
        call "!VCTOOLSBAT!" x86
        set "VCVARS_CALLED=1"
    )
    if not defined VCVARS_CALLED if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" (
        set "VCTOOLSBAT=%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat"
        echo Calling "!VCTOOLSBAT!" x86...
        call "!VCTOOLSBAT!" x86
        set "VCVARS_CALLED=1"
    )
    if not defined VCVARS_CALLED if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvarsall.bat" (
        set "VCTOOLSBAT=%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvarsall.bat"
        echo Calling "!VCTOOLSBAT!" x86...
        call "!VCTOOLSBAT!" x86
        set "VCVARS_CALLED=1"
    )
    if not defined VCVARS_CALLED if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" (
        set "VCTOOLSBAT=%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvarsall.bat"
        echo Calling "!VCTOOLSBAT!" x86...
        call "!VCTOOLSBAT!" x86
        set "VCVARS_CALLED=1"
    )

    rem Final check for cl.exe after attempted initialization
    where cl >nul 2>nul
    if errorlevel 1 (
        echo cl.exe was still not found.
        echo Open a Visual Studio x86 Native Tools Command Prompt, or install the Visual C++ build tools, then run build.bat again.
        exit /b 1
    )
)

if not exist build mkdir build
if not exist build\obj mkdir build\obj
if not exist assets\textures\wood.jpg (
    echo Missing required texture:
    echo   assets\textures\wood.jpg
    echo Add a wood texture there, then run build.bat again.
    exit /b 1
)

set SOURCES=
for /R src %%f in (*.cpp) do (
    set SOURCES=!SOURCES! "%%f"
)

cl /nologo /W4 /EHsc /std:c++17 /MD /DWIN32 /D_WINDOWS /DNOMINMAX /I"%DX_INC%" !SOURCES! ^
    /Fo:build\obj\ ^
    /Fe:build\Sigma.exe ^
    /link /LIBPATH:"%DX_LIB%" ^
    /MACHINE:X86 /SUBSYSTEM:WINDOWS ^
    d3d9.lib d3dx9.lib dxguid.lib user32.lib gdi32.lib winmm.lib

if errorlevel 1 (
    echo Build failed.
    exit /b 1
)

xcopy assets build\assets /E /I /Y >nul
if errorlevel 2 (
    echo Failed to copy runtime assets into build\assets.
    exit /b 1
)

echo.
echo Build succeeded: build\Sigma.exe
echo Runtime assets copied to: build\assets
echo Run: build\Sigma.exe

endlocal
