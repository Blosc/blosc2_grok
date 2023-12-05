/*********************************************************************
Blosc - Blocked Shuffling and Compression Library

Copyright (C) 2023  The Blosc Developers <blosc@blosc.org>
    https://blosc.org
            License: GNU Affero General Public License v3.0 (see LICENSE.txt)

                          Test program demonstrating use of the Blosc filter from C code.
                      Compile this program with cmake and run:

    $ ./test_grok2
    image size width 256, height 256
        Compress OK:	cratio: 2.891 x
                      Going to decompress
                      Decompressing buffer

                      Image Info
                      Width: 256
                      Height: 256
                      Number of components: 3
                      Precision of component 0 : 8
                      Precision of component 1 : 8
                      Precision of component 2 : 8
                      Number of tiles: 1
                      Component 0 : dimensions (256,256) at precision 8
                                    Component 1 : dimensions (256,256) at precision 8
                                                  Component 2 : dimensions (256,256) at precision 8
                                                                Decompress OK

                                                                    **********************************************************************/

#include <cmath>
#include <cstdio>

#include "b2nd.h"
#include "blosc2.h"
#include "blosc2_grok.h"
#include "grok.h"
#include "utils.h"

                                                                int comp_decomp(bool reg) {
    const char *filename = "/Users/martaiborra/blosc2_grok/examples/kodim23.ppm";
    const char *outFile = "/Users/martaiborra/blosc2_grok/src/test.jp2";
    uint64_t compressedLength = 0;
    PPMImage *img = readPPM(filename);

    const uint32_t dimX = img->x;
    const uint32_t dimY = img->y;
    // const uint32_t precision = 8;

    // initialize compress parameters
    grk_cparameters compressParams;
    grk_compress_set_default_params(&compressParams);
    compressParams.cod_format = GRK_FMT_JP2;
    compressParams.verbose = true;
    compressParams.allocationByRateDistoration = true;
    compressParams.layer_rate[0] = 5.0;
    compressParams.numlayers = 1;
    printf("dp compress set def params\n");

    grk_stream_params streamParams;
    grk_set_default_stream_params(&streamParams);
    printf("dp grk_set_default_stream_params\n");

    // b2nd
    int8_t ndim = 3;
    int64_t shape[] = {3, dimX, dimY};
    int32_t chunkshape[] = {3, (int32_t)dimX, (int32_t)dimY};
    int32_t blockshape[] = {3, (int32_t)dimX, (int32_t)dimY};
    uint8_t itemsize = 1;

    int64_t bufLen =  3 * dimX * dimY * itemsize; // buflen for b2nd
    uint8_t *c_buffer = (uint8_t *)malloc(bufLen);
    get_cbuffer(img, c_buffer);
    // memcpy(c_buffer, img->data, bufLen);

    blosc2_cparams cparams = BLOSC2_CPARAMS_DEFAULTS;
    cparams.compcode = 160;
    // cparams.compcode = BLOSC_BLOSCLZ;
    // cparams.compcode = BLOSC_ZSTD;
    // cparams.clevel = 9;
    //  Register grok codec
    if (reg) {
        blosc2_codec grok_codec = {0};
        grok_codec.compname = (char *)"grok";
        grok_codec.compcode = 160;
        grok_codec.complib = 1;
        grok_codec.version = 0;
        grok_codec.encoder = NULL;
        grok_codec.decoder = NULL;
        int rc = blosc2_register_codec(&grok_codec);
        if (rc < 0) {
            printf("Error registering codec\n");
            return -1;
        }
    }


    cparams.typesize = itemsize;
    for (int i = 0; i < BLOSC2_MAX_FILTERS; i++) {
        cparams.filters[i] = 0;
    }

    // Codec parameters
    blosc2_grok_params codec_params = {0};
    codec_params.compressParams = compressParams;
    codec_params.streamParams = streamParams;
    cparams.codec_params = &codec_params;

    blosc2_dparams dparams = BLOSC2_DPARAMS_DEFAULTS;
    dparams.nthreads = 1;
    blosc2_storage b2_storage = {.cparams = &cparams, .dparams = &dparams};

    b2nd_context_t *ctx = b2nd_create_ctx(&b2_storage, ndim, shape, chunkshape, blockshape, NULL, 0, NULL, 0);
    printf("dp b2nd_create_ctx\n");

    b2nd_array_t *arr;
    BLOSC_ERROR(b2nd_from_cbuffer(ctx, &arr, c_buffer, bufLen));
    printf("dp b2nd_from_cbuffer\n");
    if ((arr->sc->nbytes <= 0) || (arr->sc->nbytes > bufLen)) {
        printf("Compression error\n");
        return -1;
    }
    printf("Compress OK:\t");
    printf("cratio: %.3f x\n", (float)arr->sc->nbytes / (float)arr->sc->cbytes);

    // Decompress
    printf("Going to decompress\n");
    uint8_t *buffer;
    uint64_t buffer_size = itemsize;
    for (int i = 0; i < arr->ndim; ++i) {
        buffer_size *= arr->shape[i];
    }
    buffer = static_cast<uint8_t *>(malloc(buffer_size));

    BLOSC_ERROR(b2nd_to_cbuffer(arr, buffer, buffer_size));

    // Check that the decompressed data is ok
    uint16_t *srcbuf_ptr = (uint16_t *)c_buffer;
    uint16_t *dstbuf_ptr = (uint16_t *)buffer;

    /*for (int i = 0; i < (buffer_size / itemsize); i++) {
        if (srcbuf_ptr[i] != dstbuf_ptr[i]) {
            printf("Decompressed data differs from original! In elem %d expected %d found %d\n", i, srcbuf_ptr[i],
                   dstbuf_ptr[i]);
            return -1;
        }
    }*/

    printf("Decompress OK\n");

beach:
    // cleanup
    BLOSC_ERROR(b2nd_free_ctx(ctx));
    BLOSC_ERROR(b2nd_free(arr));
    free(buffer);

    return BLOSC2_ERROR_SUCCESS;
}

int main(void) {
    // Initialization
    blosc2_init();
    blosc2_grok_init(0, true);

    int error = comp_decomp(true);
    if (error == 0) {
        printf("Try again\n");
        error = comp_decomp(false);
    }

    blosc2_grok_destroy();
    blosc2_destroy();
    return error;
}