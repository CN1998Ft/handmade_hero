@echo off

if not exist ..\build (
mkdir ..\build
echo * > ..\build\.gitignore
)

if "%~1" == "clean" (
    rmdir /s /q ..\build
) else (
    pushd "..\build\"
    cl.exe -DHANDMADE_WIN32=1 -Zi -FC ..\code\win32_handmade.cpp user32.lib Gdi32.lib
    popd
)
