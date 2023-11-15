/*********************************************************************
    Blosc - Blocked Shuffling and Compression Library

    Copyright (C) 2023  The Blosc Developers <blosc@blosc.org>
    https://blosc.org
    License: GNU Affero General Public License v3.0 (see LICENSE.txt)

    Test program demonstrating use of the Blosc filter from C code.
    Compile this program with cmake and run:

    $ ./test_grok
    Compress    OK
    Decompress  OK

**********************************************************************/

#include <cmath>
#include <cstdio>
#include <string>

#include "blosc2.h"
#include "b2nd.h"
#include "grok.h"
#include "blosc2_grok.h"


void errorCallback(const char* msg, [[maybe_unused]] void* client_data)
{
    auto t = std::string(msg) + "\n";
    fprintf(stderr, t.c_str());
}
void warningCallback(const char* msg, [[maybe_unused]] void* client_data)
{
    auto t = std::string(msg) + "\n";
    fprintf(stdout, t.c_str());
}
void infoCallback(const char* msg, [[maybe_unused]] void* client_data)
{
    auto t = std::string(msg) + "\n";
    fprintf(stdout, t.c_str());
}


int comp_decomp() {
    const uint32_t dimX = 640;
    const uint32_t dimY = 480;
    const uint16_t numComps = 1;
    int8_t ndim = 3;
    int64_t shape[] = {numComps, dimX, dimY};
    int32_t chunkshape[] = {numComps, (int32_t) dimX, (int32_t) dimY};
    int32_t blockshape[] = {numComps, (int32_t) dimX, (int32_t) dimY};
    uint8_t itemsize = 2;

    // initialize compress parameters
    grk_cparameters compressParams;
    grk_compress_set_default_params(&compressParams);
    compressParams.cod_format = GRK_FMT_JP2;
    compressParams.verbose = true;

    grk_stream_params streamParams;
    grk_set_default_stream_params(&streamParams);

    int64_t bufLen = numComps * dimX * dimY * itemsize;
    auto *image = (uint16_t*)malloc(bufLen);
    int comp_size = dimX * dimY;
    for (int compno = 0; compno < numComps; ++compno) {
        for (int i = 0; i < dimX * dimY; ++i) {
            image[compno * comp_size + i] = compno * comp_size + (int16_t)(cos(i) + 1);
        }
    }

    // set library message handlers
    grk_set_msg_handlers(infoCallback, nullptr,
                         warningCallback, nullptr,
                         errorCallback, nullptr);

    blosc2_cparams cparams = BLOSC2_CPARAMS_DEFAULTS;
    cparams.compcode = 160;
    //cparams.compcode = BLOSC_BLOSCLZ;
    //cparams.compcode = BLOSC_ZSTD;
    //cparams.clevel = 9;
    blosc2_codec grok_codec = {0};
    grok_codec.compname = (char*)"grok";
    grok_codec.compcode = 160;
    grok_codec.complib = 1;
    grok_codec.version = 0;
    grok_codec.encoder = blosc2_grok_encoder;
    grok_codec.decoder = blosc2_grok_decoder;
    int rc = blosc2_register_codec(&grok_codec);
    if (rc < 0) {
        printf("Error registering codec\n");
        return -1;
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
    blosc2_storage b2_storage = {.cparams=&cparams, .dparams=&dparams};

    b2nd_context_t *ctx = b2nd_create_ctx(&b2_storage, ndim, shape, chunkshape, blockshape,
                                          NULL, 0, NULL, 0);

    b2nd_array_t *arr;
    BLOSC_ERROR(b2nd_from_cbuffer(ctx, &arr, image, bufLen));
    if((arr->sc->nbytes <= 0) || (arr->sc->nbytes > bufLen)) {
        printf("Compression error\n");
        return -1;
    }
    printf("Compress OK:\t");
    printf("cratio: %.3f x\n", (float)arr->sc->nbytes / (float)arr->sc->cbytes);

    // Decompress
    uint16_t *buffer;
    uint64_t buffer_size = itemsize;
    for (int i = 0; i < arr->ndim; ++i) {
        buffer_size *= arr->shape[i];
    }
    buffer = (uint16_t*)malloc(buffer_size);

    BLOSC_ERROR(b2nd_to_cbuffer(arr, buffer, buffer_size));
    printf("Decompress OK\n");

    for (int i = 0; i < (buffer_size / itemsize); i++) {
        if (image[i] != buffer[i]) {
            printf("Error: decompressed data differs from original!\n");
            printf("  position: %d, value in original image: %d, value in output: %d", i, image[i], buffer[i]);
            return -1;
        }
    }

beach:
  // cleanup
  BLOSC_ERROR(b2nd_free_ctx(ctx));
  BLOSC_ERROR(b2nd_free(arr));
  free(image);
  free(buffer);

  return BLOSC2_ERROR_SUCCESS;
}


int main(void) {
  // Initialization
  blosc2_init();
  blosc2_grok_init(0, true);

  int error = comp_decomp();

  blosc2_grok_destroy();
  blosc2_destroy();
  return error;
}
