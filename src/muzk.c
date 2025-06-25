#include "muzk.h"
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <raylib.h>
#include <string.h>

#define N (1<<15)

float in[N];
float in2[N];
float complex out[N];
float max_amp;

typedef struct{
    Music song;
    const char *label;
}Muzk;

Muzk *muzk = NULL;

void fft(float in[], size_t stride, float complex out[], size_t n)
{
    if (n==1){
        out[0] = in[0];
        return;
    }
    size_t ret = n/2;
    fft(in , stride*2, out, ret);
    fft(in + stride , stride*2, out + ret , ret);
    for (size_t k = 0; k < ret; k++){
        float complex T = cexp(-I*2*PI*k/n)*out[ret+k];
        float complex e = out[k];
        out[k]          = e + T;
        out[k+ret]      = e - T;
    }
}

void callback(void *bufferData, unsigned int frames)
{
    float (*samplez)[2] = bufferData;
    for (size_t i = 0; i < frames; ++i) {
        memmove(in,in+1,(N-1)*sizeof(in[0]));
        in[N-1] = samplez[i][0];
    }
}

float amp(float complex z)
{
    return logf(z*conjf(z));
}

void muzk_init(const char *file_path)
{
    muzk = malloc(sizeof(Muzk));
    if (file_path == NULL) {
        muzk->label = "Drag&Drop Music Here";
    }else{
        muzk->label = "Now: "; 
    }
    muzk->song = LoadMusicStream(file_path);
    SetMusicVolume(muzk->song, 0.5f);
    PlayMusicStream(muzk->song);
    AttachAudioStreamProcessor(muzk->song.stream, callback);
}

Muzk* muzk_pre_reload()
{
    StopMusicStream(muzk->song);
    DetachAudioStreamProcessor(muzk->song.stream, callback);
    return muzk;
}

void muzk_post_reload(Muzk *state)
{
    muzk = state;
    PlayMusicStream(muzk->song);
    AttachAudioStreamProcessor(muzk->song.stream, callback);
}

void muzk_update()
{
    UpdateMusicStream(muzk->song);

    if (IsKeyPressed(KEY_SPACE)){
        if (IsMusicStreamPlaying(muzk->song)){
            PauseMusicStream(muzk->song);
        }else {
            PlayMusicStream(muzk->song);
        }
    }
    
    if (IsKeyPressed(KEY_Q)){
        StopMusicStream(muzk->song);
        PlayMusicStream(muzk->song);
    }

    if (IsFileDropped()) {
        FilePathList filename = LoadDroppedFiles();
        char* song_n = filename.paths[0];
        DetachAudioStreamProcessor(muzk->song.stream, callback);
        StopMusicStream(muzk->song);
        UnloadMusicStream(muzk->song);
        muzk->song = LoadMusicStream(song_n);
        PlayMusicStream(muzk->song);
        UnloadDroppedFiles(filename);
        AttachAudioStreamProcessor(muzk->song.stream, callback);
    }

    int h = GetRenderHeight();
    int w = GetRenderWidth();

    BeginDrawing();
    ClearBackground(CLITERAL(Color){0x18,0x18,0x18,0xff});

    float bar_width = (float)w/120; // Hard coded this because their are only ~120 (hearable)distinct notes 

    float step = 1.06;              // or powf(2.0f, (float)1/12);
    float base = 27.5;              // A0 (1st Music note)

    if (IsMusicValid(muzk->song)){
        if (IsMusicStreamPlaying(muzk->song)){
            for (size_t i = 0; i <N; i++) {
                float t = (float)i/N;
                float hann = 0.5 - 0.5*cosf(2*PI*t);
                in2[i] = in[i]*hann;
            }
            fft(in2, 1, out, N);
        }

        max_amp = 0.0f;
        for (size_t z = 0; z < N/2; ++z) {
            float a = amp(out[z]);  
            if (a > max_amp) max_amp = a;   
        }

        // Converts FFT output to logrithmic scale and renders bars
        for (size_t i = 0; i < 120; i++) { 
            float srt;
            float end;
            if (i==0){
                srt = 1.0f;
                end = base;
            }else{
                srt = base;
                end = (base*step);
            }

            float a = 0.0f;
            for (size_t z = srt; z < N/2 && z < (size_t)end; z++) {
                float b = amp(out[z]);  
                if (a < b) a = b;   
            }

            if (i!=0) base *=step;
            float y = a/max_amp;
            DrawRectangle(bar_width*i, h-h/2*y, bar_width, h/2*y, BLUE);
        }
    }else {
        DrawText(muzk->label, 0, 0, 69, WHITE); 
    }

    EndDrawing();
}





