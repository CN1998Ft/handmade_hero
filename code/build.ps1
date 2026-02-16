if (-Not (Test-Path ../build)){
    mkdir ../build | Out-Null
}
pushd ../build/
cl -Zi ../code/win_handmade.cpp user32.lib
popd
