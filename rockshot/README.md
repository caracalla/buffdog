# rockshot

*Short for "rocket sho(o)ter"*

This is my total beginner attempt at making a 3D game engine.
* `renderer` has the guts of the 3D rasterizer.
* `triangle` has the gross triangle drawing code.
  * `fillShaded()` is the pretty straightforward triangle drawing algorithm for sequential (single-threaded) rendering.
  * `fillBarycentric()` is my stab at something that could be parallelizable (h/t to Sokolov on this).
* `scene` handles entities and their models, physics, and generally tracking the "world" and the entities within it.
* `player` handles player movement and actions (like shooting rockets).
* `level` tracks the static world model.  It's very naive, and will eventually be replaced with something BSP tree-based or something.

## Setup (UNIX)
1. Follow setup steps in the root directory README
1. `cd` back into this directory and `make run`

## Setup (Windows)
* NOTE: **THIS IS BROKEN**.  I moved everything into `rockshot`, but I need to fix the windows build process.
1. Install SDL2 as shown in [this guide](http://lazyfoo.net/tutorials/SDL/01_hello_SDL/windows/msvsnet2010u/index.php)
    * TODO: find a less painful way to do this.  There must be a better way, but the SDL docs don't seem to tell you how to build SDL2 on Windows.
1. Open the project in Visual Studio (I used 2019)
    * Currently, only the main `buffdog` project is configured to build on Windows.

## TODO
* basic physics (collisions, angular momentum)
