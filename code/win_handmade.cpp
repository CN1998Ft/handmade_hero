#include <windows.h>
#include <stdint.h>

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

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

global_variable bool Running;

global_variable BITMAPINFO BitmapInfo;
global_variable void* BitmapMemory;

global_variable int BitmapWidth;
global_variable int BitmapHeight;
int BytesPerPixel = 4;

internal void RenderWeirdGradient(int XOffset, int YOffset){
    int Width = BitmapWidth;
    int Height = BitmapHeight;
    int Pitch = Width*BytesPerPixel;
    uint8 *Row = (uint8 *)BitmapMemory;
    for (int Y = 0; Y < BitmapHeight; ++Y){
        uint32 *Pixel = (uint32 *)Row;
        for (int X = 0; X < BitmapWidth; ++X){
            /*
            Little ENDIAN architecture.
            0x 00 00 00 00
            */
            uint8 Blue = (X + XOffset);
            uint8 Green = (Y + YOffset);

            /*
                Memory: BB GG RR xx
                Register: xx RR GG BB
             */

            *Pixel++ = ((Green << 8) | Blue);
        }
        Row += Pitch;
    }
}

internal void
Win32ResizeDIBSection(int Width, int Height){

    if(BitmapMemory) {
        VirtualFree(BitmapMemory, 0, MEM_RELEASE);
    }

    BitmapWidth = Width;
    BitmapHeight = Height;

    BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
    BitmapInfo.bmiHeader.biWidth = BitmapWidth;
    BitmapInfo.bmiHeader.biHeight = -BitmapHeight;
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = 32;
    BitmapInfo.bmiHeader.biCompression = BI_RGB;
    // BitmapInfo.bmiHeader.biSizeImage = 0;
    // BitmapInfo.bmiHeader.biXPelsPerMeter = 0;
    // BitmapInfo.bmiHeader.biYPelsPerMeter = 0;
    // BitmapInfo.bmiHeader.biClrUsed = 0;
    // BitmapInfo.bmiHeader.biClrImportant= 0;
    int BitmapMemorySize = (BitmapWidth * BitmapHeight) * BytesPerPixel;
    BitmapMemory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
}

internal void
Win32UpdateWindow(HDC DeviceContext, RECT *ClientRect, int x, int y,
                  int Width, int Height){
    int WindowWidth = ClientRect->right - ClientRect->left;
    int WindowHeight = ClientRect->bottom - ClientRect->top;

    StretchDIBits(
            DeviceContext,
            /*
            x, y, Width, Height,
            x, y, Width, Height,
            */
            0, 0, BitmapWidth, BitmapHeight,
            0, 0, WindowWidth, WindowHeight,
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
                int Width = ClientRect.right - ClientRect.left;
                int Height = ClientRect.bottom - ClientRect.top;
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

                RECT ClientRect;
                GetClientRect(Window, &ClientRect);

                PAINTSTRUCT Paint;
                HDC DeviceContext = BeginPaint(Window, &Paint);
                int x = Paint.rcPaint.left;
                int y = Paint.rcPaint.top;
                int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
                int Width = Paint.rcPaint.right - Paint.rcPaint.left;
                Win32UpdateWindow(DeviceContext, &ClientRect, x, y, Width, Height);
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
        HWND Window = CreateWindowEx(
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
        if (Window){
            Running = true;
            int XOffset = 0;
            int YOffset = 0;
            while(Running){
                MSG Message;
                while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE)){
                    if (Message.message == WM_QUIT){
                        Running = false;
                    }
                    TranslateMessage(&Message);
                    DispatchMessageA(&Message);
                }
                RenderWeirdGradient(XOffset, YOffset);
                HDC DeviceContext = GetDC(Window);
                RECT ClientRect;
                GetClientRect(Window, &ClientRect);
                int WindowWidth = ClientRect.right - ClientRect.left;
                int WindowHeight = ClientRect.bottom - ClientRect.top;
                Win32UpdateWindow(DeviceContext, &ClientRect, 0, 0, WindowWidth, WindowHeight);
                ReleaseDC(Window, DeviceContext);
                ++XOffset;
            }
        } else {
        }
    } else {
    }
    return 0;
}
