#pragma once
#ifndef _BETWEEN
#define _BETWEEN

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>

#define DEFAULT_IMAGE_0 "image0.jpg"
#define DEFAULT_IMAGE_1 "image1.jpg"
#define MAX_CORNERS 100
#define N_CORNERS 500
#define WINDOW_SIZE 15

// For convenience...
typedef IplImage Image;

/* Function Prototypes */
int between_frames(int argc, char* argv[]);

#endif // _BETWEEN