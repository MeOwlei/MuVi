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
    muzk_init(file_path);

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

