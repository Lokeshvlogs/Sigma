@echo off
setlocal

pushd "%~dp0"

set "EXE=build\Sigma.exe"

if not exist "%EXE%" (
    echo Sigma executable was not found:
    echo   %EXE%
    echo Run build.bat first.
    popd
    exit /b 1
)

"%EXE%"
set "RESULT=%ERRORLEVEL%"

popd
exit /b %RESULT%
