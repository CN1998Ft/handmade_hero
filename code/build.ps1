# Probably use pwsh for building in windows
if ($args.count -eq 0)
{
    if (-Not (Test-Path ../build)){
        mkdir ../build | Out-Null
    }
    pushd ../build/
    cl -Zi -FC ../code/win_handmade.cpp user32.lib Gdi32.lib
    popd
}
elseif (($args.count -eq 1) -and ($args[0] -eq "clean"))
{
    pushd ../build/
    Remove-Item ./*.ilk, ./*.pdb, ./*.exe, ./*.obj, ./*.rdi;
    popd
}
