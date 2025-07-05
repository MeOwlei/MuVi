#include "muzk.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <raylib.h>
#include <rlgl.h>
#include <string.h>

#define N (1<<15)
#define FONT_SIZE 69
#define M 120

float in[N];
float in2[N];
float complex out[N];
float out_smooth[M];
float out_smear[M];
float max_amp;

typedef struct{
    Music song;
    Font font;
    const char *label;
    bool error;
    Shader head;
    Shader tail;
}Muzk;

Color lcoler;
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

void muzk_init(void)
{
    muzk = malloc(sizeof(Muzk));
    assert(muzk != NULL);
    memset(muzk, 0, sizeof(*muzk));
    muzk->error = false;
    muzk->font = LoadFontEx("./fonts/OpenDyslexic/OpenDyslexicNerdFont-Regular.otf", FONT_SIZE, NULL, 0);
    if (!IsFontValid(muzk->font)) {
        UnloadFont(muzk->font);
        muzk->font = GetFontDefault();
        muzk->font.baseSize = 69;
    }
    muzk->head = LoadShader(NULL, "./shaders/head.fs");
    muzk->tail = LoadShader(NULL, "./shaders/tail.fs");
}

Muzk* muzk_pre_reload(void)
{
    if (IsMusicValid(muzk->song)) {
        DetachAudioStreamProcessor(muzk->song.stream, callback);
    }
    return muzk;
}

void muzk_post_reload(Muzk *state)
{
    muzk = state;
    if (IsMusicValid(muzk->song)) {
        AttachAudioStreamProcessor(muzk->song.stream, callback);
    }   
    UnloadShader(muzk->head);
    muzk->head = LoadShader(NULL, "./shaders/head.fs");
    UnloadShader(muzk->tail);
    muzk->tail = LoadShader(NULL, "./shaders/tail.fs");
}

void muzk_update(void)
{
    if (IsMusicValid(muzk->song)) {
        UpdateMusicStream(muzk->song);
    }

    if (IsKeyPressed(KEY_SPACE)){
        if (IsMusicValid(muzk->song)) {
            if (IsMusicStreamPlaying(muzk->song)){
                PauseMusicStream(muzk->song);
            }else {
                PlayMusicStream(muzk->song);
            }
        }
    }
    
    if (IsKeyPressed(KEY_Q)){
        if (IsMusicValid(muzk->song)) {
            StopMusicStream(muzk->song);
            PlayMusicStream(muzk->song);
        }
    }

    if (IsFileDropped()) {
        FilePathList filename = LoadDroppedFiles();
        char* song_n = filename.paths[0];
        if (IsMusicValid(muzk->song)) {
            DetachAudioStreamProcessor(muzk->song.stream, callback);
            StopMusicStream(muzk->song);
            UnloadMusicStream(muzk->song);
        }
        muzk->song = LoadMusicStream(song_n);
        if (IsMusicValid(muzk->song)) {
            PlayMusicStream(muzk->song);
            AttachAudioStreamProcessor(muzk->song.stream, callback);
        } else {
            muzk->error = true;
            UnloadMusicStream(muzk->song);
        }
        UnloadDroppedFiles(filename);
    }

    int h = GetRenderHeight();
    int w = GetRenderWidth();
    float dt = GetFrameTime();
    float tt = GetMusicTimeLength(muzk->song);

    BeginDrawing();
    ClearBackground(CLITERAL(Color){0x18,0x18,0x18,0xff});

    float bar_width = (float)w/M; // Hard coded this because their are only ~M (hearable)distinct notes 

    float step = 1.06;              // or powf(2.0f, (float)1/12);
    float base = 27.5;              // A0 (1st Music note)

    if (IsMusicValid(muzk->song)){
        muzk->error = false;
        if (IsMusicStreamPlaying(muzk->song)){
            for (size_t i = 0; i <N; i++) {
                float t = (float)i/(N-1);
                float hann = 0.5 - 0.5*cosf(2*PI*t);
                in2[i] = in[i]*hann;
            }
            fft(in2, 1, out, N);

            max_amp = 0.0f;
            for (size_t z = 0; z < N/2; ++z) {
                float a = amp(out[z]);  
                if (a > max_amp) max_amp = a;   
            }
        }

            float smoothness = 7;
            float smearness = 5;

            float sat = 0.7f;
            float val = 1.0f;
            // Converts FFT output to logrithmic scale and renders bars
            for (size_t i = 0; i < M; i++) { 
                float srt;
                float end;
                if (i==0){
                    srt = 1.0f;
                    end = base;
                }else{
                    srt = base;
                    end = base*step;
                }

                float a = 0.0f;
                for (size_t z = srt; z < N/2 && z < (size_t)end; z++) {
                    float b = amp(out[z]);  
                    if (a < b) a = b;   
                }
                base = end;
                a/=max_amp;

                // IDK why it was nan after setting every [i] = 0.0f
                if (isnan(out_smooth[i])) out_smooth[i] = 0.0f; // safeguard aganist NAN
                out_smooth[i] += (a - out_smooth[i])*dt*smoothness;
                if (isnan(out_smear[i])) out_smear[i] = 0.0f; // safeguard aganist NAN
                out_smear[i] += (out_smooth[i] - out_smear[i])*dt*smearness;

                float y = out_smooth[i];
                float hue = (float)360*i/M;
                Color c = ColorFromHSV( hue, sat, val);
                Vector2 srtpnt = {
                    bar_width*i + bar_width/2,
                    h - h*2/3*y 
                };
                Vector2 endpnt = {
                    bar_width*i + bar_width/2,
                    h 
                };
                float thick = bar_width*sqrtf(y)/2; 
                DrawLineEx(srtpnt, endpnt, thick, c);
            }

            Texture2D texture = { rlGetTextureIdDefault(), 1,1,1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
            // Rendering Head
            BeginShaderMode(muzk->head);
            for (size_t i = 0; i < M; i++) {
                float hue = (float)360*i/M;
                float y = out_smooth[i];
                Color c = ColorFromHSV( hue, sat, val);
                Vector2 center = {
                    bar_width*i + bar_width/2,
                    h - h*2/3*y
                };
                float radius = 6*bar_width*sqrtf(y); 
                Vector2 position = {
                    .x = center.x - radius,
                    .y = center.y - radius,
                };
                DrawTextureEx( texture, position, 0, 2*radius, c);
            }
            EndShaderMode();

            // Rendering Tail
            BeginShaderMode(muzk->tail);
            for (size_t i = 0; i < M; i++) {
                float srt = out_smear[i];
                float end = out_smooth[i];
                float hue = (float)360*i/M;
                Color c = ColorFromHSV( hue, sat, val);
                float height = h*2/3;
                float x = bar_width*(i + 0.5);
                float radius = 3*bar_width*sqrtf(srt); 
                Rectangle rec2;
                Vector2 pos ={.x=0,.y=0};
                Rectangle rec = {
                    .x = x - radius/2,
                    .y = 0,
                    .width = radius,
                    .height = height*(end-srt),
                };
                if (srt >= end){
                    rec.y = h - height*srt;
                    rec2 = (Rectangle){0,0,1,0.5};
                } else {
                    rec.y = h- height*end;
                    rec2 = (Rectangle){0,0.5,1,0.5};
                }
                DrawTexturePro(texture, rec2, rec, pos, 0, c);
            }
            EndShaderMode();

        float progress = GetMusicTimePlayed(muzk->song)/tt;
        DrawRectangle(0, 0, w*progress, 10, CLITERAL(Color){0x22,0x07,0x92,0xFF});
    }else {
        if (muzk->error) {
            muzk->label = "Could Not load File";
            lcoler = RED;
        }else {
            muzk->label = "Drag&Drop Music Here";
            lcoler = WHITE;
        }
        Vector2 size = MeasureTextEx(muzk->font,muzk->label, muzk->font.baseSize, 0);
        Vector2 pos = {
            w/2-size.x/2,
            h/2-size.y/2
        };
        DrawTextEx(muzk->font, muzk->label, pos, muzk->font.baseSize, 0, lcoler); 
    }

    EndDrawing();
}





