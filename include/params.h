/*
	============================================================================
	Name        : preesm.h
	Author      : kdesnos
	Version     : 1.0
	Copyright   : CECILL-C, IETR, INSA Rennes
	Description : Usefull declarations for all headers used in preesm.
	============================================================================
*/

#ifndef PARAM_H
#define PARAM_H


#define NB_FRAME 210
#define HEIGHT 270
#define WIDTH 480

#define MIN_DISPARITY -8
#define MAX_DISPARITY 8
#define NB_ITERATIONS 3

// FPS: number of frame between two FPS measures
// Should be a divisor of NB_FRAMES
#define FPS 70

typedef unsigned char uchar;

#define PATH_LEFT "./dat/BBB_3D_L.yuv"
#define PATH_RIGHT "./dat/BBB_3D_R.yuv"
#define PATH_TTF "./dat/DejaVuSans.ttf"

#define PATH_TTF_FULL PROJECT_ROOT_PATH "/" PATH_TTF
#define PATH_LEFT_FULL PROJECT_ROOT_PATH "/" PATH_LEFT
#define PATH_RIGHT_FULL PROJECT_ROOT_PATH "/" PATH_RIGHT


#endif
