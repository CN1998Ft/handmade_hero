#include <windows.h>

// int WINAPI WinMain(
//         HINSTANCE hInstance,
//         HINSTANCE hPrevInstance,
//         PSTR lpCmdLine,
//         int nCmdShow) {
//     MessageBoxA(0, "This is the handmade Hero", "Handmade Hero",
//               MB_OKCANCEL | MB_ICONINFORMATION);
//     return 0;
// }

#define internal static
#define local_persist static
#define global_variable static

global_variable bool Running;

global_variable BITMAPINFO BitmapInfo;
global_variable void* BitmapMemory;
global_variable HBITMAP BitmapHandle;
global_variable HDC BitmapDeviceContext;


internal void
Win32ResizeDIBSection(int Width, int Height){

    if (BitmapHandle){
        DeleteObject(BitmapHandle);
    }
    if (!BitmapDeviceContext) {
        BitmapDeviceContext = CreateCompatibleDC(0);
    }

    BITMAPINFO BitmapInfo;
    BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
    BitmapInfo.bmiHeader.biWidth = Width;
    BitmapInfo.bmiHeader.biHeight = Height;
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = 32;
    BitmapInfo.bmiHeader.biCompression = BI_RGB;
    BitmapInfo.bmiHeader.biSizeImage = 0;
    BitmapInfo.bmiHeader.biXPelsPerMeter = 0;
    BitmapInfo.bmiHeader.biYPelsPerMeter = 0;
    BitmapInfo.bmiHeader.biClrUsed = 0;
    BitmapInfo.bmiHeader.biClrImportant= 0;

    BitmapHandle = CreateDIBSection(
            BitmapDeviceContext, &BitmapInfo,
            DIB_RGB_COLORS,
            &BitmapMemory,
            0, 0);
}

internal void
Win32UpdateWindow(HDC DeviceContext, int x, int y, int Width, int Height){
    StretchDIBits(
            DeviceContext,
            x, y, Width, Height,
            x, y, Width, Height,
            BitmapMemory,
            &BitmapInfo,
            DIB_RGB_COLORS,
            SRCCOPY
            );
}


LRESULT CALLBACK Win32MainWindowCallback(
        HWND Window,
        UINT Message,
        WPARAM WParam,
        LPARAM LParam
        ){
    LRESULT Result = 0;

    switch(Message){
        case WM_SIZE:
            {
                RECT ClientRect;
                GetClientRect(Window, &ClientRect);
                int Height = ClientRect.right - ClientRect.left;
                int Width = ClientRect.bottom - ClientRect.top;
                Win32ResizeDIBSection(Width, Height);
                OutputDebugStringA("WM_SIZE.\n");
            } break;
        case WM_DESTROY:
            {
                // TODO: Handle this as an error. Recreate window?
                Running = false;
            } break;
        case WM_CLOSE:
            {
                // TODO: Handle this with a message to the user?
                Running = false;
            } break;
        case WM_ACTIVATEAPP:
            {
                OutputDebugStringA("WM_ACTIVATEAPP.\n");
            } break;
        case WM_PAINT:
            {
                PAINTSTRUCT Paint;
                HDC DeviceContext = BeginPaint(Window, &Paint);
                int x = Paint.rcPaint.left;
                int y = Paint.rcPaint.top;
                int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
                int Width = Paint.rcPaint.right - Paint.rcPaint.left;
                Win32UpdateWindow(DeviceContext, x, y, Width, Height);
                EndPaint(Window, &Paint);
            } break;
        default:
            {
                // OutputDebugStringA("default.\n");
                Result = DefWindowProc(Window, Message, WParam, LParam);
            } break;
    }
    return Result;
}

int CALLBACK
WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow){
    WNDCLASS WindowClass = {};
    WindowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance = hInstance;
    // WindowClass.hIcon = ;
    WindowClass.lpszClassName = "HandmadeHeroWindowClass";
    if (RegisterClass(&WindowClass)){
        HWND WindowHandle = CreateWindowEx(
                0,
                WindowClass.lpszClassName,
                "Handmade Hero",
                WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                0,
                0,
                hInstance,
                0
                );
        if (WindowHandle){
            MSG Message;
            Running = true;
            while(Running){
            BOOL MessageResult = GetMessage(&Message, 0, 0, 0);
                if (MessageResult > 0){
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                } else {
                    break;
                }
            }
        } else {
        }
    } else {
    }
    return 0;
}
