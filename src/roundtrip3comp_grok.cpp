/*********************************************************************
Blosc - Blocked Shuffling and Compression Library

Copyright (C) 2023  The Blosc Developers <blosc@blosc.org>
    https://blosc.org
            License: GNU Affero General Public License v3.0 (see LICENSE.txt)

                          Test program demonstrating use of the Blosc filter from C code.
                      Compile this program with cmake and run:

$ ./test_grok2
image size width 768, height 512
Calling grk_compress
grk_compress ended
Compress OK
Proceed to 2nd compression
image size width 768, height 512
Calling grk_compress
grk_compress ended
Compress OK

**********************************************************************/

#include <cmath>
#include <cstdio>

#include <memory>

#include "grok.h"
#include "utils.h"

int comp_decomp() {
    const char *filename = "/Users/martaiborra/blosc2_grok/examples/kodim23.ppm";
    const char *outFile = "/Users/martaiborra/blosc2_grok/src/test.jp2";
    uint64_t compressedLength = 0;
    PPMImage *img = readPPM(filename);

    const uint32_t dimX = img->x;
    const uint32_t dimY = img->y;
    const uint32_t precision = 8;
    const uint32_t numComps = 3;



    // initialize compress parameters
    grk_cparameters compressParams;
    grk_compress_set_default_params(&compressParams);
    compressParams.cod_format = GRK_FMT_JP2;
    compressParams.verbose = true;
    compressParams.allocationByRateDistoration = true;
    compressParams.layer_rate[0] = 5.0;
    compressParams.numlayers = 1;

    grk_stream_params streamParams;
    grk_set_default_stream_params(&streamParams);

    int64_t bufLen =  3 * dimX * dimY;
    uint8_t *c_buffer = (uint8_t *)malloc(bufLen);
    get_cbuffer(img, c_buffer);
    // memcpy(c_buffer, img->data, bufLen);

    grk_codec* codec = nullptr;
    std::unique_ptr<uint8_t[]> data;
    data = std::make_unique<uint8_t[]>(bufLen);
    streamParams.buf = data.get();
    streamParams.buf_len = bufLen;

    // create image from input
    auto* components = new grk_image_comp[numComps];
    for(uint32_t i = 0; i < numComps; ++i) {
        auto c = components + i;
        c->w = dimX;
        c->h = dimY;
        c->dx = 1;
        c->dy = 1;
        c->prec = precision;
        c->sgnd = false;
    }
    grk_image* image;
    if (numComps == 1) {
        image = grk_image_new(
            numComps, components, GRK_CLRSPC_GRAY, true);

    } else {
        image = grk_image_new(
            numComps, components, GRK_CLRSPC_SRGB, true);
    }

    // fill in component data
    // see grok.h header for full details of image structure
    auto *ptr = (uint8_t*)c_buffer;
    int typesize = 1;

    for (uint16_t compno = 0; compno < image->numcomps; ++compno) {
        auto comp = image->comps + compno;
        auto compWidth = comp->w;
        auto compHeight = comp->h;
        auto compData = comp->data;
        if (!compData) {
            fprintf(stderr, "Image has null data for component %d\n", compno);
            delete[] components;
            grk_object_unref(codec);
            grk_object_unref(&image->obj);
            return -1;
        }
        // fill in component data, taking component stride into account
        auto srcData = new int32_t[compWidth * compHeight];
        for (uint32_t j = 0; j < compHeight; ++j) {
            for (uint32_t i = 0; i < compWidth; ++i) {
                memcpy(srcData + j * compWidth + i, ptr, typesize);
                ptr += typesize;
            }
        }

        auto srcPtr = srcData;
        for (uint32_t j = 0; j < compHeight; ++j) {
            memcpy(compData, srcPtr, compWidth * sizeof(int32_t));
            srcPtr += compWidth;
            compData += comp->stride;
        }
        delete[] srcData;
    }

    // initialize compressor
    int size = -1;
    codec = grk_compress_init(&streamParams, &compressParams, image);
    if (!codec) {
        fprintf(stderr, "Failed to initialize compressor\n");
        goto beach;
    }

    // compress
    printf("Calling grk_compress\n");
    size = (int)grk_compress(codec, nullptr);
    printf("grk_compress ended\n");
    if (size == 0) {
        size = -1;
        fprintf(stderr, "Failed to compress\n");
        goto beach;
    }

beach:
    // cleanup
    free(c_buffer);
    delete[] components;
    grk_object_unref(codec);
    grk_object_unref(&image->obj);
    printf("Compress OK\n");
    if(size >= 0) {
        return 0;
    }
    return size;

}

int main(void) {
    // Initialize grok
    grk_initialize(nullptr, 0, true);
    int error = comp_decomp();
    if(error >= 0) {
        printf("Proceed to 2nd compression\n");
        error = comp_decomp();
    }
    grk_deinitialize();

    return error;
}