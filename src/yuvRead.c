/*
	============================================================================
	Name        : readYUV.c
	Author      : kdesnos
    Author      : mpelcat
	Version     : 1.0
	Copyright   : CECILL-C
	Description :
	============================================================================
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "yuvRead.h"
#include "clock.h"


/*========================================================================

   Global Variable

   ======================================================================*/
#define NB_PATH 2
char* path[] = {PATH_LEFT_FULL,PATH_RIGHT_FULL};

static FILE * ptfile[NB_PATH] ;
/*========================================================================

   initReadYUV DEFINITION

   ======================================================================*/
void initReadYUV(int id, int xSize, int ySize) {
    int fsize;
    if((ptfile[id] = fopen(path[id], "rb")) == NULL )
    {
        fprintf(stderr,"ERROR: Task read cannot open yuv_file '%s'\n", path[id]);
        return;
    }

#ifdef PREESM_VERBOSE
    printf("Opened file '%s'\n", path[id]);
#endif

    // Obtain file size:
    fseek (ptfile[id] , 0 , SEEK_END);
    fsize = ftell (ptfile[id]);
    rewind (ptfile[id]);
    if(fsize < NB_FRAME*(xSize*ySize + xSize*ySize/2))
    {
        fprintf(stderr,"ERROR: Task read yuv_file incorrect size");
        //system("PAUSE");
        //return;
    }

#ifdef PREESM_VERBOSE
    printf("Correct size for yuv_file '%s'\n", path[id]);
#endif

    // Set initial clock
    startTiming(0);
}

/*========================================================================

   readYUV DEFINITION

   ======================================================================*/
void readYUV(int id, int xSize, int ySize, unsigned char *y, unsigned char *u, unsigned char *v) {

    static int frame[2] = { 0, 0 };
    if (frame[id] >= NB_FRAME) {
        rewind(ptfile[id]);
        frame[id] = 0;
    }
    frame[id]++;

	if(id == 1 && ftell(ptfile[id])%(FPS*(xSize*ySize + xSize*ySize/2)) == 0){
        static int first = 1;
        if (first != 1) {
            unsigned int time = 0;
            time = stopTiming(0);
            printf("\nMain: %d frames in %d us - %f fps\n", FPS, time, ((float)FPS) / (float)time * 1000000);
        }
        first = 0;
        startTiming(0);
    }

    size_t res = fread(y, sizeof(char), xSize * ySize, ptfile[id]);
    res += fread(u, sizeof(char), xSize * ySize / 4, ptfile[id]);
    res += fread(v, sizeof(char), xSize * ySize / 4, ptfile[id]);
    if (res <= 0) {
     	printf("Error while reading input file\n");
    }
}
