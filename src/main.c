#include <assert.h>
#include <stdio.h>
#include <raylib.h>
#include <dlfcn.h>
#include <string.h>

#include "muzk.h"

const char *lib_name = "libmuzk.so";
void *libmuzk = NULL;
muzk_init_t muzk_init = NULL;
muzk_update_t muzk_update = NULL;
Muzk muzk = {0};

#define ARRAY_LEN(xs) sizeof(xs)/sizeof(xs[0])

size_t globe_frames_count = 0;


char* shift_arg(int *argc, char ***argv)
{
    assert(*argc > 0);
    char *result = (**argv);
    (*argv) +=1;
    (*argc) -=1;
    return result;
}
bool reload_lib()
{
    if (libmuzk != NULL) dlclose(libmuzk);
    libmuzk = dlopen(lib_name, RTLD_NOW);
    if (libmuzk == NULL){
        fprintf(stderr, "ERROR: %s", dlerror());
        return 1;
    }
    muzk_init = dlsym(libmuzk, "muzk_init");
    if (muzk_init == NULL){
        fprintf(stderr, "ERROR: %s", dlerror());
        return 1;
    }
    muzk_update = dlsym(libmuzk, "muzk_update");
    if (muzk_update == NULL){
        fprintf(stderr, "ERROR: %s", dlerror());
        return 1;
    }
    return true;
}

int main(int argc, char **argv){

    const char *program = shift_arg(&argc, &argv);

    if (argc == 0) {
        fprintf(stderr, "USSAGE: %s <input>\n", program);
        fprintf(stderr, "ERROR: no input file provided.\n");
        return 1;
    }
    if (!reload_lib()) return 1;

    const char *file_path = shift_arg(&argc, &argv);

    InitWindow(840, 630, "Music Visualizeir");
    SetTargetFPS(60);
    InitAudioDevice();
    muzk_init(&muzk, file_path);

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_R)) {
            reload_lib();
        }
        muzk_update(&muzk);
    }
    CloseWindow();

    return 0;
}

