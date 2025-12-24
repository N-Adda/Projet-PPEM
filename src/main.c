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

	int nproc = omp_get_num_procs(); //nombre de processeurs logiques
	omp_set_dynamic(0); //empêcher runtime d'ajuster automatiquement
	omp_set_num_threads(nproc - 1);
	omp_set_nested(0); // Désactiver nested parallelism pour éviter oversubscription accidentelle

	printf("Stereo Matching App\n");

	//Calcul du Speedup 
	double speedup = 0.0;
	double temp_1_thread = 19733; //Temps d'execution avec 1 thread en ms
	double t_frame_start = 0.0, t_frame_end = 0.0;
	double T_frame_acc = 0.0;
	int frameCount = 0;

	//Variable temps rgb2gray
	//double t_census_start, t_census_end;
	//double T_census = 0.0;


	// Open YUV Files (left & right)
	initReadYUV(0, WIDTH, HEIGHT);
	initReadYUV(1, WIDTH, HEIGHT);

	// Init display
	displayRGBInit(0, HEIGHT, WIDTH);
	displayRGBInit(1, HEIGHT, WIDTH);



	while (!stopThreads) {
		frameCount++;
		if (frameCount == 1) {
			t_frame_start = omp_get_wtime();
		}

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
		
		static float grayL[HEIGHT * WIDTH], grayR[HEIGHT * WIDTH];
		rgb2Gray(HEIGHT * WIDTH, rgbL, grayL);
		rgb2Gray(HEIGHT * WIDTH, rgbR, grayR);
		

		// Census
		static unsigned char cenL[HEIGHT * WIDTH], cenR[HEIGHT * WIDTH];
		census(HEIGHT, WIDTH, grayL, cenL);
		census(HEIGHT, WIDTH, grayR, cenR);
		
		// Pre-compute weights for offset aggregation
		int offsets[NB_ITERATIONS];
		static float weightsHor[NB_ITERATIONS * HEIGHT * WIDTH * 3], weightsVert[NB_ITERATIONS * HEIGHT * WIDTH * 3];
		offsetGen(NB_ITERATIONS, offsets);
		for (unsigned idx = 0; idx < NB_ITERATIONS; idx++) {
			//t_census_start = omp_get_wtime();
			computeWeights(HEIGHT, WIDTH, 0, offsets + idx, rgbL, weightsHor + idx * (3 * HEIGHT * WIDTH));
			computeWeights(HEIGHT, WIDTH, 1, offsets + idx, rgbL, weightsVert + idx * (3 * HEIGHT * WIDTH));
			//t_census_end = omp_get_wtime();
			//T_census += (t_census_end - t_census_start);
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

		/*frameCount++;

		if (frameCount % 30 == 0) {
			//printf("Temps moyen disparitySelect = %.3f ms\n",
				//(T_disp / frameCount) * 1000.0);

			printf("Temps computeweight = %.3f ms\n",
				(T_census / frameCount) * 1000.0);
		}*/

		if (frameCount == 210) {
			t_frame_end = omp_get_wtime();
			T_frame_acc = t_frame_end - t_frame_start;
			speedup = temp_1_thread / (T_frame_acc * 1000.0);
			printf("Le Speedup est = %.6f \n", (speedup));
		
		
		
		}

	}

	return 0;

}

