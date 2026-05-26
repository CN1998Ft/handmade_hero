#include <windows.h>
#include <stdint.h>
#include <xinput.h>
#include <dsound.h>

#define internal static
#define local_persist static
#define global_variable static

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

struct win32_offscreen_buffer {
    BITMAPINFO Info;
    void* Memory;

    int Width;
    int Height;

    int Pitch;
    int BytesPerPixel = 4;
};

struct win32_window_dimension {
    int Width;
    int Height;
};

#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub){
    return ERROR_DEVICE_NOT_CONNECTED;
}
global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;

#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub){
    return ERROR_DEVICE_NOT_CONNECTED;
}
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;

#define XInputGetState XInputGetState_
#define XInputSetState XInputSetState_

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter);
typedef DIRECT_SOUND_CREATE(direct_sound_create);


internal void Win32LoadXInput(void){
    HMODULE XInputLibrary = LoadLibraryA("xinput1_3.dll");
    if (XInputLibrary){
        XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary,"XInputGetState");
        XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
    }
}

global_variable bool GlobalRunning;
global_variable win32_offscreen_buffer GlobalBackBuffer;
global_variable LPDIRECTSOUNDBUFFER GlobalSecondaryBuffer;

internal void Win32InitDSound(HWND Window, int32 SamplesPerSecond, int32 BufferSize){

    HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");

    if (DSoundLibrary) {
        direct_sound_create *DirectSoundCreate = (direct_sound_create *)
            GetProcAddress(DSoundLibrary, "DirectSoundCreate");
        LPDIRECTSOUND DirectSound;
        if (DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0))){

                WAVEFORMATEX Waveformat = {};
                Waveformat.wFormatTag = WAVE_FORMAT_PCM;
                Waveformat.nChannels = 2;
                Waveformat.nSamplesPerSec = SamplesPerSecond;
                Waveformat.wBitsPerSample = 16;
                Waveformat.nBlockAlign = (Waveformat.nChannels * Waveformat.wBitsPerSample) / 8;
                Waveformat.nAvgBytesPerSec = Waveformat.nSamplesPerSec * Waveformat.nBlockAlign;
                Waveformat.cbSize = 0;

                if (SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY))){

                    DSBUFFERDESC BufferDescription = {};
                    BufferDescription.dwSize = sizeof(BufferDescription);
                    BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

                    LPDIRECTSOUNDBUFFER PrimaryBuffer;
                    if (SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &PrimaryBuffer, 0))){

                        HRESULT Error = PrimaryBuffer->SetFormat(&Waveformat);
                        if (SUCCEEDED(Error)){
                            OutputDebugStringA("Primary Buffer format was set.\n");
                        // if (SUCCEEDED(PrimaryBuffer->SetFormat(&Waveformat))){
                        } else {
                        }
                    } else {
                    }
                }
                DSBUFFERDESC BufferDescription = {};
                BufferDescription.dwSize = sizeof(BufferDescription);
                BufferDescription.dwFlags = 0;
                BufferDescription.dwBufferBytes = BufferSize;
                BufferDescription.lpwfxFormat = &Waveformat;
                HRESULT Error =DirectSound->CreateSoundBuffer(&BufferDescription, &GlobalSecondaryBuffer, 0);
                if (SUCCEEDED(Error)){
                    OutputDebugStringA("Secondary Buffer was set properly");
                }
                BufferDescription.dwBufferBytes = BufferSize;
        } else{
            }
    } else {
            }
}

internal win32_window_dimension Win32GetWindowDimension(HWND Window){
    win32_window_dimension Result;
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Width = ClientRect.right - ClientRect.left;
    Result.Height = ClientRect.bottom - ClientRect.top;

    return Result;
}

internal void RenderWeirdGradient(win32_offscreen_buffer *Buffer,
        int XOffset, int YOffset){

    uint8 *Row = (uint8 *)Buffer->Memory;
    for (int Y = 0; Y < Buffer->Height; ++Y){
        uint32 *Pixel = (uint32 *)Row;
        for (int X = 0; X < Buffer->Width; ++X){
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
        Row += Buffer->Pitch;
    }
}

internal void
Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height){

    if(Buffer->Memory) {
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }

    Buffer->Width = Width;
    Buffer->Height = Height;
    Buffer->BytesPerPixel = 4;

    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;
    int BitmapMemorySize = (Buffer->Width * Buffer->Height) * Buffer->BytesPerPixel;
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

    Buffer->Pitch = Width*Buffer->BytesPerPixel;

}

internal void
Win32DisplayBufferInWindow(HDC DeviceContext, int WindowWidth, int WindowHeight,
        win32_offscreen_buffer *Buffer,
        int x, int y, int Width, int Height){

    StretchDIBits(
            DeviceContext,
            /*
            x, y, Width, Height,
            x, y, Width, Height,
            */
            0, 0, WindowWidth, WindowHeight,
            0, 0, Buffer->Width, Buffer->Height,
            Buffer->Memory,
            &Buffer->Info,
            DIB_RGB_COLORS,
            SRCCOPY
            );
}


internal LRESULT CALLBACK Win32MainWindowCallback(
        HWND Window,
        UINT Message,
        WPARAM WParam,
        LPARAM LParam
        ){
    LRESULT Result = 0;

    switch(Message){
        case WM_DESTROY:
            {
                // TODO: Handle this as an error. Recreate window?
                GlobalRunning = false;
            } break;

        case WM_CLOSE:
            {
                // TODO: Handle this with a message to the user?
                GlobalRunning = false;
            } break;

        case WM_ACTIVATEAPP:
            {
                OutputDebugStringA("WM_ACTIVATEAPP.\n");
            } break;

        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
            {
                uint32 VKCode = WParam;
                bool WasDown = ((LParam & (1 << 30)) != 0);
                bool IsDown = ((LParam & (1 << 31)) == 0);
                if (WasDown != IsDown){
                    if (VKCode == 'W'){
                    }else if (VKCode == 'A'){
                    }else if (VKCode == 'S'){
                    }else if (VKCode == 'D'){
                    }else if (VKCode == 'Q'){
                    }else if (VKCode == 'E'){
                    }else if (VKCode == VK_UP){
                    }else if (VKCode == VK_DOWN){
                    }else if (VKCode == VK_LEFT){
                    }else if (VKCode == VK_RIGHT){
                    }else if (VKCode == VK_ESCAPE){
                        OutputDebugStringA("ESCAPE: ");
                        if (IsDown){
                            OutputDebugStringA("IsDown ");
                            }
                        if (WasDown) {
                            OutputDebugStringA("WasDown ");
                        }
                        OutputDebugStringA("\n");
                    }else if (VKCode == VK_SPACE){
                    }
                }

                bool AltKeyWasDown = ((LParam & (1 << 29)) != 0);
                if ((VKCode == VK_F4) && AltKeyWasDown){
                    GlobalRunning = false;
                }
            } break;

        case WM_PAINT:
            {
                PAINTSTRUCT Paint;
                HDC DeviceContext = BeginPaint(Window, &Paint);
                int x = Paint.rcPaint.left;
                int y = Paint.rcPaint.top;
                win32_window_dimension Dimension = Win32GetWindowDimension(Window);
                Win32DisplayBufferInWindow(DeviceContext, Dimension.Width,
                        Dimension.Height,
                        &GlobalBackBuffer,
                        x, y, Dimension.Width, Dimension.Height);
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
    HINSTANCE Instance,
    HINSTANCE PrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow){

    Win32LoadXInput();

    WNDCLASSA WindowClass = {};

    Win32ResizeDIBSection(&GlobalBackBuffer, 1000, 800);

    WindowClass.style = CS_HREDRAW|CS_VREDRAW;
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance = Instance;
    // WindowClass.hIcon = ;
    WindowClass.lpszClassName = "HandmadeHeroWindowClass";
    if (RegisterClass(&WindowClass)){
        HWND Window = CreateWindowEx(
                0,
                WindowClass.lpszClassName,
                // "Handmade Hero",
                "Motion ",
                WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                // 1000,
                // 800,
                0,
                0,
                Instance,
                0
                );
        if (Window){

            int XOffset = 0;
            int YOffset = 0;

            int SamplesPerSecond = 48000;
            int ToneHz = 256;
            int16 ToneVolume = 3000;
            uint32 RunningSampleIndex = 0;
            int SquareWavePeriod = SamplesPerSecond/ToneHz;
            int HalfSquareWavePeriod = SquareWavePeriod/2;
            int BytesPerSample = sizeof(int16)*2;
            int SecondaryBufferSize = SamplesPerSecond*BytesPerSample;

            Win32InitDSound(Window, SamplesPerSecond, SecondaryBufferSize);

            GlobalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);

            GlobalRunning = true;
            // float XOffset = 0.0f;
            // float YOffset = 0.0f;

            while(GlobalRunning){
                HDC DeviceContext = GetDC(Window);
                MSG Message;
                while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE)){
                    if (Message.message == WM_QUIT){
                        GlobalRunning = false;
                    }
                    TranslateMessage(&Message);
                    DispatchMessageA(&Message);
                }

                // Get the gamepad connected.
                for (DWORD ControllerIndex = 0;
                    ControllerIndex < XUSER_MAX_COUNT;
                    ++ControllerIndex){
                    XINPUT_STATE ControllerState;
                    if (XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS){

                        XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;

                        // Gamepad buttons setup
                        bool Up = Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP;
                        bool Down = Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
                        bool Left = Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
                        bool Right = Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;
                        bool Start = Pad->wButtons & XINPUT_GAMEPAD_START;
                        bool Back = Pad->wButtons & XINPUT_GAMEPAD_BACK;
                        bool LeftShoulder = Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER;
                        bool RightShoulder = Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER;
                        bool AButton = Pad->wButtons & XINPUT_GAMEPAD_A;
                        bool BButton = Pad->wButtons & XINPUT_GAMEPAD_B;
                        bool XButton = Pad->wButtons & XINPUT_GAMEPAD_X;
                        bool YButton = Pad->wButtons & XINPUT_GAMEPAD_Y;

                        // Sticks
                        int16 StickX = Pad->sThumbLX;
                        int16 StickY = Pad->sThumbLY;
                        if (AButton) {
                            YOffset += 2;
                        }
                    } else {
                    }
                }
                //
                // XINPUT_VIBRATION Vibration;
                // Vibration.wLeftMotorSpeed = 6000;
                // Vibration.wRightMotorSpeed = 6000;
                // XInputSetState(0, &Vibration);

                RenderWeirdGradient(&GlobalBackBuffer, XOffset, YOffset);

                DWORD PlayCursor;
                DWORD WriteCursor;
                if (SUCCEEDED(GlobalSecondaryBuffer->GetCurrentPosition(
                                &PlayCursor,
                                &WriteCursor
                                ))){
                    DWORD ByteToLock = RunningSampleIndex*BytesPerSample % SecondaryBufferSize;
                    DWORD BytesToWrite;
                    if (ByteToLock == PlayCursor){
                        BytesToWrite = SecondaryBufferSize;
                    }
                    else if (ByteToLock > PlayCursor){
                        BytesToWrite = SecondaryBufferSize - ByteToLock;
                        BytesToWrite += PlayCursor;
                    } else {
                        BytesToWrite = PlayCursor - ByteToLock;
                    }

                    VOID *Region1;
                    DWORD Region1Size;
                    VOID *Region2;
                    DWORD Region2Size;
                    if(SUCCEEDED(GlobalSecondaryBuffer->Lock(ByteToLock, BytesToWrite,
                                                &Region1, &Region1Size,
                                                &Region2, &Region2Size,
                                                0))){

                        int16 *SampleOut = (int16 *)Region1;
                        DWORD Region1SampleCount = Region1Size/BytesPerSample;
                        for (DWORD SampleIndex = 0;
                             SampleIndex < Region1SampleCount;
                             ++SampleIndex){

                            int16 SampleValue = ((RunningSampleIndex++ / HalfSquareWavePeriod) % 2 )? ToneVolume : -ToneVolume;
                            *SampleOut++ = SampleValue;
                            *SampleOut++ = SampleValue;
                        }

                        SampleOut = (int16 *)Region2;
                        DWORD Region2SampleCount = Region2Size/BytesPerSample;
                        for (DWORD SampleIndex = 0;
                             SampleIndex < Region2SampleCount;
                             ++SampleIndex){

                            int16 SampleValue = ((RunningSampleIndex++ / HalfSquareWavePeriod) % 2 )? ToneVolume : -ToneVolume;
                            *SampleOut++ = SampleValue;
                            *SampleOut++ = SampleValue;
                        }
                        GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
                    }
                }

                win32_window_dimension Dimension = Win32GetWindowDimension(Window);
                Win32DisplayBufferInWindow(DeviceContext, Dimension.Width, Dimension.Height,
                        &GlobalBackBuffer,
                        0, 0, Dimension.Width, Dimension.Height);
                ReleaseDC(Window, DeviceContext);
                ++XOffset;
                YOffset -= 1;
                // XOffset += 0.5f;
                // YOffset += 0.5f;
            }
        } else {
        }
    } else {
    }
    return 0;
}
