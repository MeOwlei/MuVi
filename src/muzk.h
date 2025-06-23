#ifndef MUZK_H_

#define MUZK_H_ 
#include <raylib.h>

typedef struct{
    Music song;
}Muzk;

typedef void (*muzk_init_t)(void *, const char*) ;
typedef void (*muzk_update_t)(void *) ;

#endif // MUZK_H_


