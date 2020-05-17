#ifndef BUFFDOG_TEXTURE
#define BUFFDOG_TEXTURE

#include "vector.h"

struct Texture {
	virtual Vector vectorColorFromUV(double u, double v) = 0;
};

#endif
