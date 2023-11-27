#include <string>
#include <vector>

#include "utils.h"


PPMImage *readPPM(const char *filename)
{
    char buff[16];
    PPMImage *img;
    FILE *fp;
    int c, rgb_comp_color;
    //open PPM file for reading
    fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "Unable to open file '%s'\n", filename);
        exit(1);
    }

    //read image format
    if (!fgets(buff, sizeof(buff), fp)) {
        perror(filename);
        exit(1);
    }

    //check the image format
    if (buff[0] != 'P' || buff[1] != '6') {
        fprintf(stderr, "Invalid image format (must be 'P6')\n");
        exit(1);
    }

    //alloc memory form image
    img = (PPMImage *)malloc(sizeof(PPMImage));
    if (!img) {
        fprintf(stderr, "Unable to allocate memory\n");
        exit(1);
    }

    //check for comments
    c = getc(fp);
    while (c == '#') {
        while (getc(fp) != '\n') ;
        c = getc(fp);
    }

    ungetc(c, fp);
    //read image size information
    if (fscanf(fp, "%d %d", &img->x, &img->y) != 2) {
        fprintf(stderr, "Invalid image size (error loading '%s')\n", filename);
        exit(1);
    }
    printf("image size width %d, height %d\n", img->x, img->y);

    //read rgb component
    if (fscanf(fp, "%d", &rgb_comp_color) != 1) {
        fprintf(stderr, "Invalid rgb component (error loading '%s')\n", filename);
        exit(1);
    }

    //check rgb component depth
    /* if (rgb_comp_color!= RGB_COMPONENT_COLOR) {
         fprintf(stderr, "'%s' does not have 8-bits components\n", filename);
         exit(1);
     }*/

    while (fgetc(fp) != '\n') ;
    //memory allocation for pixel data
    img->data = (PPMPixel*)malloc(img->x * img->y * sizeof(PPMPixel));

    if (!img) {
        fprintf(stderr, "Unable to allocate memory\n");
        exit(1);
    }

    //read pixel data from file
    if (fread(img->data, 3 * img->x, img->y, fp) != img->y) {
        fprintf(stderr, "Error loading image '%s'\n", filename);
        exit(1);
    }

    fclose(fp);
    img->grayscale = (uint16_t *)malloc(img->x * img->y * sizeof(uint16_t));
    for (int i = 0; i < img->x * img->y; ++i) {
        img->grayscale[i] = (uint16_t)(img->data[i].red + img->data[i].green + img->data[i].blue) / 3;
    }
    return img;
}


int get_cbuffer(PPMImage* img, uint8_t *c_buffer) {
    uint8_t *comp0 = (uint8_t *)malloc(img->x * img->y);
    uint8_t *comp1 = (uint8_t *)malloc(img->x * img->y);
    uint8_t *comp2 = (uint8_t *)malloc(img->x * img->y);
    for (int i = 0; i < img->x * img->y; ++i) {
        comp0[i] = img->data[i].red;
        comp1[i] = img->data[i].green;
        comp2[i] = img->data[i].blue;
    }
    memcpy(c_buffer, comp0, img->x * img->y);
    memcpy(c_buffer + img->x * img->y, comp1, img->x * img->y);
    memcpy(c_buffer + 2 * img->x * img->y, comp2, img->x * img->y);

    free(comp0);
    free(comp1);
    free(comp2);
    return 0;
}