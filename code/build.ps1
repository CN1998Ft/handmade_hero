if (-Not (Test-Path ../build)){
    mkdir ../build
}
pushd ../build/
pwd
cl -Zi ../code/win_handmade.cpp user32.lib
popd
