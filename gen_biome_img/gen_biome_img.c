// generate an image of the world
#include "../generator.h"
#include "../util.h"
#include "../finders.h"
#include <time.h>
#include <stdio.h>

const int stronghold_sprite[5][5] =
        {
                {0x000000, 0x1e4835, 0x102c31, 0x102c31, 0x000000},
                {0x1e4835, 0x88c35a, 0x88c35a, 0x88c35a, 0x102c31},
                {0x1e4835, 0xb2e25c, 0x1b2b10, 0x88c35a, 0x1e4835},
                {0x316364, 0xb2e25c, 0xb2e25c, 0x88c35a, 0x1e4835},
                {0x000000, 0x316364, 0x316364, 0x1e4835, 0x000000}
        };

// Draw a 5x5 sprite at the position of a stronghold
void draw_stronghold(unsigned char *pixels, Pos stronghold_pos, Range r, int pixscale) {
    // Find positions of top-left positions of the sprite
    int start_x = stronghold_pos.x - 2 - r.x;
    int start_z = stronghold_pos.z - 2 - r.z;

    for (int i = -1; i < 6; i++) {
        int x = start_x + i;
        // Do not go out of bounds
        if (x < 0) continue;
        if (x > r.sx) break;
        for (int j = -1; j < 6; j++) {
            int z = start_z + j;
            // Do not go out of bounds
            if (z < 0) continue;
            if (z > r.sz) break;
            int pix = 0;
            if (j >= 0 && i >= 0 && j < 5 && i < 5) { // In range for the sprite, otherwise border
                pix = stronghold_sprite[j][i];
            }
            for (int m = 0; m < pixscale; m++) {
                for (int n = 0; n < pixscale; n++) {
                    int idx = pixscale * x + n;
                    idx += (r.sx * pixscale) * ((pixscale * (r.sz - 1 - z)) + m);
                    idx *= 3;

                    pixels[idx] = (unsigned char) ((pix >> 16) & 0xFF);
                    pixels[idx + 1] = (unsigned char) ((pix >> 8) & 0xFF);
                    pixels[idx + 2] = (unsigned char) (pix & 0xFF);
                }
            }
        }
    }
}

int main() {
    Generator g;
    int mc = MC_1_18;
    int stronghold_count = 16;
    uint64_t seed = 123LL;
    int pix4cell = 4;
    Range r;
    // 1:16, a.k.a. horizontal chunk scaling
    r.scale = 16;
    // Define the position and size for a horizontal area:
    r.x = -60, r.z = -60;   // position (x,z)
    r.sx = 2048, r.sz = 2048; // size (width,height)
    // Set the vertical range as a plane near sea level at scale 1:4.
    r.y = 15, r.sy = 1;

    setupGenerator(&g, mc, LARGE_BIOMES);
    applySeed(&g, DIM_OVERWORLD, seed);

    // Allocate the necessary cache for this range.
    int *biomeIds = allocCache(&g, r);

    // Generate the area inside biomeIds, indexed as:
    // biomeIds[i_y*r.sx*r.sz + i_z*r.sx + i_x]
    // where (i_x, i_y, i_z) is a position relative to the range cuboid.
    clock_t start_time = clock();
    genBiomes(&g, biomeIds, r);
    clock_t end_time = clock();
    double elapsed_time = (end_time - start_time) / (double) CLOCKS_PER_SEC;
    printf("Biome generation took %fs\n", elapsed_time);

    // Generate 16 strongholds
    start_time = clock();
    StrongholdIter sh;
    Pos stronghold_pos[16];
    stronghold_pos[0] = initFirstStronghold(&sh, mc, seed);
    for (int i = 1; i < stronghold_count; i++) {
        if (nextStronghold(&sh, &g) <= 0)
            break;
        stronghold_pos[i] = sh.pos;
    }
    end_time = clock();
    elapsed_time = (end_time - start_time) / (double) CLOCKS_PER_SEC;
    printf("Stronghold generation took %fs\n", elapsed_time);
    printf("(strongholds found at");
    for (int i = 0; i < stronghold_count; i++) {
        printf(" (%d, %d)", stronghold_pos[i].x, stronghold_pos[i].z);
    }
    printf("\n");


    // Map the biomes to an image buffer, with 4 pixels per biome cell.
    int imgWidth = pix4cell * r.sx, imgHeight = pix4cell * r.sz;
    unsigned char biomeColors[256][3];
    initBiomeColors(biomeColors);
    unsigned char *pixels = (unsigned char *) malloc(3 * imgWidth * imgHeight);
    start_time = clock();
    biomesToImage(pixels, biomeColors, biomeIds, r.sx, r.sz, pix4cell, 2);
    for (int i = 0; i < stronghold_count; i++) {
        draw_stronghold(pixels, stronghold_pos[i], r, pix4cell);
    }
    end_time = clock();
    elapsed_time = (end_time - start_time) / (double) CLOCKS_PER_SEC;
    printf("Image generation took %fs\n", elapsed_time);

    // Save the RGB buffer to a PPM image file.
    start_time = clock();
    savePPM("map.ppm", pixels, imgWidth, imgHeight);
    end_time = clock();
    elapsed_time = (end_time - start_time) / (double) CLOCKS_PER_SEC;
    printf("Image writing took %fs\n", elapsed_time);

    // Clean up.
    free(biomeIds);
    free(pixels);

    return 0;
}