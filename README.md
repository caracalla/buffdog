# Buffdog

## TODO
* basic physics (collisions, angular momentum)
* Voronoi shattering in 3D (see more on my [blog](http://caracal.la/01_intro.html))

## Setup (UNIX)
1. Install SDL2
    ```
    git clone https://github.com/spurious/SDL-mirror.git SDL
    cd SDL
    mkdir build
    cd build
    ../configure
    make
    sudo make install
    ```
1. `cd` back into this directory and `make run`

## Setup (Windows)
1. Install SDL2 as shown in [this guide](http://lazyfoo.net/tutorials/SDL/01_hello_SDL/windows/msvsnet2010u/index.php)
    * TODO: find a less painful way to do this.  There must be a better way, but the SDL docs don't seem to tell you how to build SDL2 on Windows.
1. Open the project in Visual Studio (I used 2019)
    * Currently, only the main `buffdog` project is configured to build on Windows.

## Targets
* `buffdog` - A real-time rasterizer, that is slowly becoming a game engine.  It currently runs on a custom software renderer (SDL is only used to create a window, fill individual pixels, and handle inputs).
* `spinner` - My trusty spinning line, letting me know everything is working.
* `delaunay` - A Delaunay Triangulation demonstration, which is a stepping stone to implementing 3D Voronoi shattering.
* `voronoi` - A Voronoi diagram demonstration, the next step to shattering.

### Legacy Targets (UNIX only)
* `ray_tracer` - A rudimentary real-time ray tracer.
* `buffdog` - The real-time rasterizer, at a much earlier stage of development.

## Resources and Inspirations
* Gabriel Gambetta's [Computer Graphics from Scratch](https://www.gabrielgambetta.com/computer-graphics-from-scratch/introduction.html)
* Eric Lengyel's [Mathematics for 3D Game Programming and Computer Graphics](http://mathfor3dgameprogramming.com/)
* Peter Shirley's [Ray Tracing in One Weekend](https://raytracing.github.io/books/RayTracingInOneWeekend.html)
* [The Quake source code](https://github.com/caracalla/quake)
* [small3dlib](https://gitlab.com/drummyfish/small3dlib)
* Dmitry Sokolov's [tinyrenderer](https://github.com/ssloy/tinyrenderer/wiki/Lesson-0-getting-started)

## Namesake

https://www.youtube.com/user/buffcorrell

## Debugging

1. `make`
1. `lldb buffdog`
