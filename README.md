# MuVi
A Music Visualizer inspired by [Musializer](https://github.com/tsoding/musializer)

## Quick Start

```console
$ ./nob
$ ./build/muvi <song.mp3>
```

## Hot Reloading

Keep the app running. Rebuild with `./nob`. Hot reload by focusing on the window of the app and pressing <kbd>r</kbd>.

```console
$ export HOTRELOAD=1
$ export LD_LIBRARY_PATH="./build/:$LD_LIBRARY_PATH"
$ ./nob
$ ./build/muvi <song.mp3>
```

