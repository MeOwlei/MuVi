#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <raylib.h>
#include <string.h>
#include <complex.h>
#include <math.h>

#define ARRAY_LEN(xs) sizeof(xs)/sizeof(xs[0])
#define N 256// (1<<14)

typedef struct{
    float left;
    float right;
}Frame;

float in[N];
float complex out[N];

size_t globe_frames_count = 0;

void fft(float in[], size_t stride, float complex out[], size_t n)
{
    if (n<=1){
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
float amp(float complex z){
    float r = fabsf(crealf(z));
    float c = fabsf(cimagf(z));
    if (r > c) return r;
    return c;
}
float max_amp;
void callback(void *bufferData, unsigned int frames)
{
    if (frames > N) frames = N;
    Frame *samplez = bufferData;
    for (size_t i = 0; i < frames; i++) {
        in[i] = samplez[i].left;
    }
    fft(in, 1, out, N);
    max_amp = 0.0f;
    for (size_t i = 0; i < frames; i++) {
        // float a = cabsf(out[i]);
        float a = amp(out[i]);
        if (max_amp < a) max_amp = a;   
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
        
        float bar_width = (float)w/120; // Hard coded this because their are only ~120 (hearable)distinct notes 
        float step = powf(2.0f, (float)1/12 );
        int m = 0;
        for (float f = 27.5f; (size_t)f < N; f*=step) {
            float fn = f*step;
            float a = 0;
            for (size_t q = (size_t)f; q < N && q < (size_t)fn; q++) {
                a+=amp(out[(size_t)q]); 
            }
            a/=(size_t)fn -(size_t)f +1;
            float y = a/max_amp;
            DrawRectangle(bar_width*m, h/2 - h/2*y, bar_width, h/2*y, BLUE);
            m+=1;
        }
        // for (size_t i = 0; i < N; ++i) {
        //     float y = cabsf(out[i])/max_amp;
        //     DrawRectangle(bar_width*i, h/2 - h/2*y, bar_width, h/2*y, BLUE);
        // }

        EndDrawing();
    }
    CloseWindow();

    return 0;
}

