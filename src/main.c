#include <assert.h>
#include <stdio.h>
#include <raylib.h>
#include <dlfcn.h>
#include <string.h>

#include "muzk.h"

const char *lib_name = "libmuzk.so";
void *libmuzk = NULL;

#ifdef HOTRELOAD
#define X(name, ...) name##_t *name = NULL;
LIST_OF_FUNC
#undef X
#else
#define X(name, ...) name##_t name;
LIST_OF_FUNC
#undef X
#endif

bool reload_lib()
{
#ifdef HOTRELOAD
    if (libmuzk != NULL) dlclose(libmuzk);
    libmuzk = dlopen(lib_name, RTLD_NOW);
    if (libmuzk == NULL){
        fprintf(stderr, "ERROR: %s", dlerror());
        return 1;
    }

#define X(name, ...) name = dlsym(libmuzk, #name);   \
    if (name == NULL){                          \
        fprintf(stderr, "ERROR: %s", dlerror());\
        return 1;                               \
    }
LIST_OF_FUNC
#undef X
#endif
    return true;
}

int main(int argc, char **argv){

    if (!reload_lib()) return 1;

    InitWindow(840, 630, "Music Visualizeir");
    SetTargetFPS(60);
    InitAudioDevice();
    muzk_init();

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_R)) {
            void *state = muzk_pre_reload();
            reload_lib();
            muzk_post_reload(state);
        }
        muzk_update();
    }
    CloseWindow();

    return 0;
}

