# Probably use pwsh for building in windows
# Create build for compilation
if (-Not (Test-Path ../build)){
    mkdir ../build | Out-Null
    echo '*' > ../build/.gitignore
}

# Actual compilation process
if (($args.count -eq 1) -and ($args[0] -eq "clean"))
{
    rm -Recurse -Force ../build
}
else
{
    pushd ../build/
    cl.exe -DHANDMADE_WIN32=1 -Zi -FC ../code/win32_handmade.cpp user32.lib Gdi32.lib
    popd
}
