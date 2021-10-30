# Buffdog

## TODO
* Voronoi shattering in 3D (see more on my [blog](http://caracal.la/01_intro.html))
* Port rendering to Vulkan
* Move logging stuff to a `logging.h` or something

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

## Targets
* `spinner` - My trusty spinning line, a basic sanity check, letting me know everything is working.
* `buffdog` - Whatever I'm working on.  Right now, I'm messing around with 2D collision physics.
* `rockshot` - A real-time rasterizer, which is slowly becoming a game engine.  It currently runs on a custom software renderer (SDL is only used to create a window, fill individual pixels, and handle inputs).
* `delaunay` - A Delaunay Triangulation demonstration, which is a stepping stone to implementing 3D Voronoi shattering.
* `voronoi` - A Voronoi diagram demonstration, the next step to shattering.

## Resources and Inspirations
* Gabriel Gambetta's [Computer Graphics from Scratch](https://www.gabrielgambetta.com/computer-graphics-from-scratch/introduction.html)
* Eric Lengyel's [Mathematics for 3D Game Programming and Computer Graphics](http://mathfor3dgameprogramming.com/)
* Peter Shirley's [Ray Tracing in One Weekend](https://raytracing.github.io/books/RayTracingInOneWeekend.html)
* [The Quake source code](https://github.com/caracalla/quake)
* [small3dlib](https://gitlab.com/drummyfish/small3dlib)
* Dmitry Sokolov's [tinyrenderer](https://github.com/ssloy/tinyrenderer/wiki/Lesson-0-getting-started)

## Namesake

https://www.youtube.com/user/buffcorrell
