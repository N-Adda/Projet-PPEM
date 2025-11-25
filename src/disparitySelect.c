/*
	============================================================================
	Name        : disparitySelect.c
	Author      : kdesnos
	Version     : 1.0
	Copyright   : CeCILL-C, IETR, INSA Rennes
	Description : Iterative selection of the disparity with the lowest cost for
				  each pixel in order to construct the depth map.
	============================================================================
*/

#include "disparitySelect.h"
#include <string.h>

#define min(x,y) (((x)<(y))?(x):(y))
#define max(x,y) (((x)<(y))?(y):(x))

void disparitySelect(int height, int width, int scale,
	int minDisparity,
	char* disparity,
	float* aggregatedDisparity,
	float* bestCost,
	unsigned char* result)
{
	int i, j;


	// For all other iterations
	// Scan the pixels of the aggregated disparity
	for (j = 0; j < height; j++)
	{
		for (i = 0; i < width; i++)
		{
			// If the cost of the aggregated disparity is lower, keep the new
			// disparity as the best, else, keep te current.
			result[j * width + i] =
				(aggregatedDisparity[j * width + i] < bestCost[j * width + i]) ?
				scale * ((*disparity) - minDisparity) : result[j * width + i];

			bestCost[j * width + i] = min(aggregatedDisparity[j * width + i], bestCost[j * width + i]);

		}
	}

}
