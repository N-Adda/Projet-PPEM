#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

#include "params.h"
#include "yuvRead.h"
#include "displayRGB.h"
#include "yuv2RGB.h"
#include "rgb2Gray.h"
#include "census.h"
#include "costConstruction.h"
#include "offsetGen.h"
#include "computeWeights.h"
#include "aggregateCost.h"
#include "disparitySelect.h"
#include "medianFilter.h"
#include "md5.h"
#include <omp.h>

int stopThreads = 0;

int main(void) {

	// Empêche les parallélismes imbriqués
	//omp_set_num_threads(4);  // CPU = 4 threads logiques

	printf("Stereo Matching App\n");

	//Variable temps disparity
	/*double t_disp_start, t_disp_end;
	double T_disp = 0.0;*/
	int frameCount = 0;

	//Variable temps rgb2gray
	double t_rgb2gray_start, t_rgb2gray_end;
	double T_rgb2gray = 0.0;


	// Open YUV Files (left & right)
	initReadYUV(0, WIDTH, HEIGHT);
	initReadYUV(1, WIDTH, HEIGHT);

	// Init display
	displayRGBInit(0, HEIGHT, WIDTH);
	displayRGBInit(1, HEIGHT, WIDTH);



	while (!stopThreads) {

		// Read images
		static unsigned char yL[HEIGHT * WIDTH], uL[HEIGHT * WIDTH / 4], vL[HEIGHT * WIDTH / 4];
		static unsigned char yR[HEIGHT * WIDTH], uR[HEIGHT * WIDTH / 4], vR[HEIGHT * WIDTH / 4];
		readYUV(0, WIDTH, HEIGHT, yL, uL, vL);
		readYUV(1, WIDTH, HEIGHT, yR, uR, vR);

		// Convert images to RGB
		static unsigned char rgbL[HEIGHT * WIDTH * 3], rgbR[HEIGHT * WIDTH * 3];
		yuv2rgb(WIDTH, HEIGHT, yL, uL, vL, rgbL);
		yuv2rgb(WIDTH, HEIGHT, yR, uR, vR, rgbR);

		
		// Convert to gray
		t_rgb2gray_start = omp_get_wtime();
		static float grayL[HEIGHT * WIDTH], grayR[HEIGHT * WIDTH];
		rgb2Gray(HEIGHT * WIDTH, rgbL, grayL);
		rgb2Gray(HEIGHT * WIDTH, rgbR, grayR);
		t_rgb2gray_end = omp_get_wtime();
		T_rgb2gray += (t_rgb2gray_end - t_rgb2gray_start);

		// Census
		static unsigned char cenL[HEIGHT * WIDTH], cenR[HEIGHT * WIDTH];
		census(HEIGHT, WIDTH, grayL, cenL);
		census(HEIGHT, WIDTH, grayR, cenR);

		// Pre-compute weights for offset aggregation
		int offsets[NB_ITERATIONS];
		static float weightsHor[NB_ITERATIONS * HEIGHT * WIDTH * 3], weightsVert[NB_ITERATIONS * HEIGHT * WIDTH * 3];
		offsetGen(NB_ITERATIONS, offsets);
		for (unsigned idx = 0; idx < NB_ITERATIONS; idx++) {
			computeWeights(HEIGHT, WIDTH, 0, offsets + idx, rgbL, weightsHor + idx * (3 * HEIGHT * WIDTH));
			computeWeights(HEIGHT, WIDTH, 1, offsets + idx, rgbL, weightsVert + idx * (3 * HEIGHT * WIDTH));
		}

		// Find for each pixel, the disparity level minimizing the aggregated costs.
		static unsigned char depthMap[HEIGHT * WIDTH];
		memset(depthMap, 0, HEIGHT * WIDTH * sizeof(char));
		static float bestCost[HEIGHT * WIDTH];



		// For each degree of disparity
		for (char disp = MIN_DISPARITY; disp <= MAX_DISPARITY; disp++) {

			// Cost construction
			static float dispError[HEIGHT * WIDTH];			
			costConstruction(HEIGHT, WIDTH, 12 /*Magic number*/, &disp, grayL, grayR, cenL, cenR, dispError);
		
			static float aggregatedDisparityCost[HEIGHT * WIDTH];
			aggregateCost(HEIGHT, WIDTH, NB_ITERATIONS, dispError, offsets, weightsHor, weightsVert, aggregatedDisparityCost);

			if (disp == MIN_DISPARITY) {
				memcpy(bestCost, aggregatedDisparityCost, HEIGHT * WIDTH * sizeof(float));
			}
			else {
				//t_disp_start = omp_get_wtime();
				// Compare the current disparity cost to previous ones
				disparitySelect(HEIGHT, WIDTH, 12, MIN_DISPARITY, &disp, aggregatedDisparityCost, bestCost, depthMap);
				//t_disp_end = omp_get_wtime();
				//T_disp += (t_disp_end - t_disp_start);
			}
		}

		// Apply median filter on result
		static unsigned char filteredDepthMap[HEIGHT * WIDTH];
		medianFilter(HEIGHT, WIDTH, 1, depthMap, filteredDepthMap);

		// Display
		displayRGB(0, HEIGHT, WIDTH, rgbL);
		displayLum(1, filteredDepthMap);

		// MD5

		MD5_Update(HEIGHT * WIDTH * sizeof(char), filteredDepthMap);

		frameCount++;

		if (frameCount % 30 == 0) {
			/*printf("Temps moyen disparitySelect = %.3f ms\n",
				(T_disp / frameCount) * 1000.0);*/

			printf("Temps moyen rgb2gray = %.3f ms\n",
				(T_rgb2gray / frameCount) * 1000.0);
		}
	}

	return 0;

}

