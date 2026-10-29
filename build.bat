@echo off
setlocal

for %%a in (%*) do set "%%a=1"

if not exist build mkdir build

where cl >nul 2>nul
if errorlevel 1 (
    if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" (
        for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -products * -latest -property installationPath`) do (
            if exist "%%i\VC\Auxiliary\Build\vcvarsall.bat" (
                call "%%i\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul
            )
        )
    )
)

where cl >nul 2>nul
if errorlevel 1 (
    if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" (
        call "%ProgramFiles(x86)%\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul
    )
)

where cl >nul 2>nul
if errorlevel 1 (
    echo Error: cl.exe was not found and MSVC environment could not be initialized.
    exit /b 1
)

cl /nologo /std:clatest /Zc:preprocessor /Od /Z7 /W4 /wd4201 /wd5105 /wd4100 /Isrc ^
   src\pendulum\pendulum_main.c /Febuild\pendulum.exe /Fobuild\ /Fdbuild\pendulum.pdb ^
   /link user32.lib gdi32.lib winmm.lib

if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%

if "%run%"=="1" (
    build\pendulum.exe
)
