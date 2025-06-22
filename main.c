#include <assert.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <raylib.h>
#include <string.h>
#include <complex.h>
#include <math.h>

#include "muzk.h"

muzk_init_t muzk_init = NULL;

#define ARRAY_LEN(xs) sizeof(xs)/sizeof(xs[0])
#define N (1<<15)

float in[N];
float in2[N];
float complex out[N];
float max_amp;

size_t globe_frames_count = 0;

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


char* shift_arg(int *argc, char ***argv)
{
    assert(*argc > 0);
    char *result = (**argv);
    (*argv) +=1;
    (*argc) -=1;
    return result;
}

int main(int argc, char **argv){

    const char *program = shift_arg(&argc, &argv);

    if (argc == 0) {
        fprintf(stderr, "USSAGE: %s <input>\n", program);
        fprintf(stderr, "ERROR: no input file provided.\n");
        return 1;
    }

    const char *file_path = shift_arg(&argc, &argv);

    InitWindow(800, 600, "Music Visualizeir");
    SetTargetFPS(60);
    InitAudioDevice();
    Music song = LoadMusicStream(file_path);
    PlayMusicStream(song);
    SetMusicVolume(song, 0.5f);
    AttachAudioStreamProcessor(song.stream, callback);

    while (!WindowShouldClose()) {

        UpdateMusicStream(song);

        if (IsKeyPressed(KEY_SPACE)){
            if (IsMusicStreamPlaying(song)){
                PauseMusicStream(song);
            }else {
                PlayMusicStream(song);
            }
        }
        if (IsKeyPressed(KEY_Q)){
            StopMusicStream(song);
            PlayMusicStream(song);
        }

        int h = GetRenderHeight();
        int w = GetRenderWidth();
        
        BeginDrawing();
        ClearBackground(CLITERAL(Color){0x18,0x18,0x18,0xff});

        float bar_width = (float)w/120; // Hard coded this because their are only ~120 (hearable)distinct notes 

        float step = 1.06;              // or powf(2.0f, (float)1/12);
        float base = 27.5;              // A0 (Music note)

        if (IsMusicStreamPlaying(song)){
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
            for (size_t i = 0; i < 120; i++) { 
                float srt;
                float end;
                if (i==0){
                    srt = 1.0f;
                    end = base;
                }else{
                    srt= base;
                    end= (base*step);
                }
                float a = 0.0f;
                for (size_t z = base; z < N/2 && z < (size_t)end; z++) {
                    float b = amp(out[z]);  
                    if (a < b) a = b;   
                }
                if (i!=0) base *=step;
                float y = a/max_amp;
                DrawRectangle(bar_width*i, h-h/2*y, bar_width, h/2*y, BLUE);
            }
        // } 
        EndDrawing();
    }
    CloseWindow();

    return 0;
}

