/*********************************************************************
Blosc - Blocked Shuffling and Compression Library

Copyright (c) 2021  The Blosc Development Team <blosc@blosc.org>
https://blosc.org
License: BSD 3-Clause (see LICENSE.txt)

See LICENSE.txt for details about copyright and rights to use.
**********************************************************************/


#ifndef WRAPPER_H
#define WRAPPER_H

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

#include "blosc2.h"
#include "b2nd.h"

typedef struct {
    uint8_t qfactor;
    bool isJPH;
    uint8_t color_space;
    uint32_t dimX;
    uint32_t dimY;
    uint32_t nthreads;
    grk_cparameters compressParams;
    grk_stream_params streamParams;
} blosc2_grok_params;

int blosc2_grok_encoder(
    const uint8_t *input,
    int32_t input_len,
    uint8_t *output,
    int32_t output_len,
    uint8_t meta,
    blosc2_cparams* cparams,
    const void* chunk
);

int blosc2_grok_decoder(const uint8_t *input, int32_t input_len, uint8_t *output, int32_t output_len,
                        uint8_t meta, blosc2_dparams *dparams, const void *chunk);

void blosc2_grok_init(uint32_t nthreads, bool verbose);
void blosc2_grok_destroy();

#ifdef __cplusplus
}
#endif

#endif
