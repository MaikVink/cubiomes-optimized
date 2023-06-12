// generate an image of the world
#include "../generator.h"
#include "../util.h"
#include <time.h>
#include <stdio.h>

int main()
{
    Generator g;
    setupGenerator(&g, MC_1_18, LARGE_BIOMES);

    uint64_t seed = 123LL;
    applySeed(&g, DIM_OVERWORLD, seed);

    Range r;
    // 1:16, a.k.a. horizontal chunk scaling
    r.scale = 16;
    // Define the position and size for a horizontal area:
    r.x = -60, r.z = -60;   // position (x,z)
    r.sx = 1200, r.sz = 1200; // size (width,height)
    // Set the vertical range as a plane near sea level at scale 1:4.
    r.y = 15, r.sy = 1;

    // Allocate the necessary cache for this range.
    int *biomeIds = allocCache(&g, r);

    // Generate the area inside biomeIds, indexed as:
    // biomeIds[i_y*r.sx*r.sz + i_z*r.sx + i_x]
    // where (i_x, i_y, i_z) is a position relative to the range cuboid.
	clock_t start_time = clock();
    genBiomes(&g, biomeIds, r);
	clock_t end_time = clock();
	double elapsed_time = (end_time-start_time)/(double)CLOCKS_PER_SEC;
	printf("Biome generation took %fs\n", elapsed_time);

    // Map the biomes to an image buffer, with 4 pixels per biome cell.
    int pix4cell = 4;
    int imgWidth = pix4cell*r.sx, imgHeight = pix4cell*r.sz;
    unsigned char biomeColors[256][3];
    initBiomeColors(biomeColors);
    unsigned char *rgb = (unsigned char *) malloc(3*imgWidth*imgHeight);
	start_time = clock();
    biomesToImage(rgb, biomeColors, biomeIds, r.sx, r.sz, pix4cell, 2);
	end_time = clock();
	elapsed_time = (end_time-start_time)/(double)CLOCKS_PER_SEC;
	printf("Image generation took %fs\n", elapsed_time);
		
    // Save the RGB buffer to a PPM image file.
	start_time = clock();
    savePPM("map.ppm", rgb, imgWidth, imgHeight);
	end_time = clock();
	elapsed_time = (end_time-start_time)/(double)CLOCKS_PER_SEC;
	printf("Image writing took %fs\n", elapsed_time);

    // Clean up.
    free(biomeIds);
    free(rgb);

    return 0;
}