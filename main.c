#include <assert.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <raylib.h>
#include <string.h>
#include <complex.h>
#include <math.h>

#include "plug.h"

plug_init_t plug_init = NULL;

#define ARRAY_LEN(xs) sizeof(xs)/sizeof(xs[0])
#define N (1<<15)

typedef struct{
    float left;
    float right;
}Frame;

float in[N];
float in2[N];
float complex out[N];

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
float max_amp;
void callback(void *bufferData, unsigned int frames)
{
    if (frames > N) frames = N;
    Frame *samplez = bufferData;
    for (size_t i = 0; i < frames; i++) {
        in[i] = samplez[i].left;
    }
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
    SetExitKey(KEY_Q);
    SetMusicVolume(song, 0.5f);
    float tt = GetMusicTimeLength(song);
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

        if (IsMusicStreamPlaying(song)){
            for (size_t i = 0; i <N; i++) {
                float t = (float)i/N;
                float hann = 0.5 - 0.5*cosf(2*PI*t);
                in2[i] = in[i]*hann;
            }
            fft(in2, 1, out, N);
        }
        
        float bar_width = (float)w/120; // Hard coded this because their are only ~120 (hearable)distinct notes 
        float step = powf(2.0f, 1/12);
        float *Amps = (float *)malloc(sizeof(float)*120);
        float base = 27.5;//1.0f; 
        Amps[0] = 0.0f;
        max_amp = 0.0f;
        for (size_t i = 1; i < 120; i++) { 
            float srt;
            float end;
            if (i==0){
                srt = Amps[i];
                end = base;
            }else{
                srt= base;
                end= (base*step);
            }
            float a = 0;
            for (size_t z = base; z < N/2 && z < (size_t)end; z++) {
                a+=cabsf(out[z]);  
            }
            a/= end - srt + 1; 
            if (max_amp < a) max_amp = a;   
            Amps[i] = a;
            base *=step;
        }

        for (size_t i = 0; i < 120; i++) {
            float y = Amps[i]/max_amp;
            DrawRectangle(bar_width*i, h/2-h/2*y, bar_width, h/2*y, BLUE);
        }

        // int m = 0;
        // for (float f = 20.0f; (size_t)f < N; f*=step) {
        //     float fn = f*step;
        //     float a = 0;
        //     for (size_t q = (size_t)f; q < N && q < (size_t)fn; q++) {
        //         a+=amp(out[(size_t)q]); 
        //     }
        //    a/=(size_t)fn -(size_t)f +1; 
        //     float y = a/max_amp;
        //     DrawRectangle(bar_width*m, h/2 - h/2*y, bar_width, h/2*y, BLUE);
        //     m+=1;
        // }

        // for (size_t i = 0; i < 120; ++i) {
        //     float y = cabsf(out[i])/max_amp;
        //     DrawRectangle(bar_width*i, h/2 - h/2*y, bar_width, h/2*y, BLUE);
        // }

        EndDrawing();
    }
    CloseWindow();

    return 0;
}

