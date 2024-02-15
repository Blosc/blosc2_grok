#ifndef BLOSC2_GROK_UTILS_H
#define BLOSC2_GROK_UTILS_H

#include <stdio.h>
#include <cmath>
#include <cstdio>
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    unsigned char red, green, blue;
} PPMPixel;

typedef struct {
    int x, y;
    PPMPixel *data;
    uint16_t *grayscale;
} PPMImage;

PPMImage *readPPM(const char *filename);
int get_cbuffer(PPMImage* img, uint8_t *c_buffer);
void free_PPM(PPMImage* img);

#ifdef __cplusplus
}
#endif

#endif