/*********************************************************************
 * blosc2_grok: Grok (JPEG2000 codec) plugin for Blosc2
 *
 * Copyright (c) 2023  The Blosc Development Team <blosc@blosc.org>
 * https://blosc.org
 * License: GNU Affero General Public License v3.0 (see LICENSE.txt)
**********************************************************************/


#ifndef WRAPPER_H
#define WRAPPER_H

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

#include "blosc2.h"
#include "grok.h"
#include "b2nd.h"
#include <stdio.h>

typedef struct {
    grk_cparameters compressParams;
    grk_stream_params streamParams;
} blosc2_grok_params;

void blosc2_grok_init(uint32_t nthreads, bool verbose);
void blosc2_grok_destroy();

void blosc2_grok_set_default_params(const int64_t *tile_size, const int64_t *tile_offset,
                                    int numlayers, char *quality_mode, double *quality_layers,
                                    int numgbits, char *progression,
                                    int num_resolutions, int64_t *codeblock_size, int cblk_style,
                                    bool irreversible, int roi_compno, int roi_shift, const int64_t *precinct_size,
                                    const int64_t *offset,
                                    GRK_SUPPORTED_FILE_FMT decod_format,
                                    GRK_SUPPORTED_FILE_FMT cod_format, bool enableTilePartGeneration,
                                    int mct, int max_cs_size,
                                    int max_comp_size, int rsiz, int framerate,
                                    bool apply_icc_,
                                    GRK_RATE_CONTROL_ALGORITHM rateControlAlgorithm, int num_threads, int deviceId,
                                    int duration, int repeats,
                                    bool verbose);


#ifdef __cplusplus
}
#endif

#endif
