#include <assert.h>
#include <stdio.h>
#include <raylib.h>
#include <dlfcn.h>
#include <string.h>
#include "muzk.h"

// #define HOTRELOAD

const char *lib_name = "libmuzk.so";
void *libmuzk = NULL;

#ifdef HOTRELOAD
#define X(name, ...) name##_t *name = NULL;
#else
#define X(name, ...) name##_t name;
#endif
LIST_OF_FUNC
#undef X

#ifdef HOTRELOAD
bool reload_lib()
{
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
    return true;
}
#else 
#define reload_lib() true
#endif

int main(void){

    printf("Where is it??");
    if (!reload_lib()) return 1;

    InitWindow(840, 630, "Music Visualizeir");
    SetTargetFPS(60);
    InitAudioDevice();
    printf("Its NOT Okay");
    muzk_init();
    printf("Its Okay");

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

