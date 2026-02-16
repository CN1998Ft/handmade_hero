#include <windows.h>

// int CALLBACK
// WinMain(
//     HINSTANCE hInstance,
//     HINSTANCE hPrevInstance,
//     LPSTR lpCmdLine,
//     int nCmdShow){
//     MessageBoxA(0, "This is the handmade Hero.", "Handmade Hero",
//             MB_OK|MB_ICONINFORMATION);
//     return 0;
// }
int WINAPI 
WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    PSTR lpCmdLine,
    int nCmdShow) {
    MessageBoxA(0, "This is the handmade Hero", "Handmade Hero",
            MB_OKCANCEL|MB_ICONINFORMATION);
    return 0;
}
