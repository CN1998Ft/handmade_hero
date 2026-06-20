#if !defined(HANDMADE_H)

struct game_offscreen_buffer
{
    // BITMAPINFO Info;
    void *Memory;

    int Width;
    int Height;

    int Pitch;
};

internal void
GameUpdateAndRender(game_offscreen_buffer *Buffer, int BlueOffset, int GreenOffset);

#define HANDMADE_H
#endif
