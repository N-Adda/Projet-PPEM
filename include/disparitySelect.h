/*
	============================================================================
	Name        : disparitySelect.h
	Author      : kdesnos
	Version     : 1.0
	Copyright   : CeCILL-C, IETR, INSA Rennes
	Description : Iterative selection of the disparity with the lowest cost for
	              each pixel in order to construct the depth map.
	============================================================================
*/

#ifndef DISPARITY_SELECT_H
#define DISPARITY_SELECT_H

/**
* This function compares two costs images corresponding to different
* disparities. For each pixel, the disparity corresponding to the lowest cost
* is kept as the resulting disparity for this pixel. The best cost for each
* pixel is also outputed by the function in order to be used by iterative calls
* to the disparitySelect() function.
*
* @param[in] height
*        The height of the processed stereo pair.
* @param[in] width
*        The width of the processed stereo pair
* @param[in] scale
*        Disparities are multiplied by this scale before they are written in
*        the result array.
* @param[in] minDisparity
*        Minimum disparity that can be found.
* @param[in] disparity
*        The disparity corresponding to the costs contained in the
*        aggregatedDisparity array.
* @param[in] aggregatedDisparity
*        Height*width costs computed by aggregateCost()
* @param[in,out] result
*        Array of the disparities with the lowest costs. (size=height*width)
* @param[in,out] bestCost
*        Array of the the lowest costs (+index). (size=height*width+1)
*/
void disparitySelect (int height, int width, int scale,
                      int minDisparity,
                      char *disparity,
					  float *aggregatedDisparity,
                      float *bestCost, unsigned char *result);

#endif
