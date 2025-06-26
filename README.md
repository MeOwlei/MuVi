# MuVi
A Music Visualizer inspired by [Musializer](https://github.com/tsoding/musializer)

> [!WARNING]
> NOT Compeleted. Only works on Linux

## Dependency 

- [Raylib](https://github.com/raysan5/raylib)

## Quick Start

```console
$ cc -o build/muvi src/muzk.c src/main.c -lm -lraylib
$ ./build/muvi 
```

## Hot Reloading

Keep the app running. Rebuild with `./nob`. Hot reload by focusing on the window of the app and pressing <kbd>r</kbd>.

```console
$ export HOTRELOAD=1
$ export LD_LIBRARY_PATH="./build/:$LD_LIBRARY_PATH"
$ cc -o build/muvi  src/main.c -lm -lraylib
$ ./build/muvi
```

