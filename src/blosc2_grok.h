/*********************************************************************
Blosc - Blocked Shuffling and Compression Library

Copyright (c) 2021  The Blosc Development Team <blosc@blosc.org>
https://blosc.org
License: GNU Affero General Public License v3.0 (see LICENSE.txt)
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

void blosc2_grok_set_default_params(bool tile_size_on, int tx0, int ty0, int t_width, int t_height,
                                   int numlayers, bool allocationByRateDistoration,
                                   double *layer_rate, bool allocationByQuality, double *layer_distortion,
                                   int csty, int numgbits, GRK_PROG_ORDER prog_order,
                                   int numpocs,
                                   int numresolution, int cblockw_init, int cblockh_init, int cblk_sty,
                                   bool irreversible, int roi_compno, int roi_shift, int res_spec,
                                   int image_offset_x0, int image_offset_y0, int subsampling_dx,
                                   int subsampling_dy, GRK_SUPPORTED_FILE_FMT decod_format,
                                   GRK_SUPPORTED_FILE_FMT cod_format, bool enableTilePartGeneration,
                                   int newTilePartProgressionDivider, int mct, int max_cs_size,
                                   int max_comp_size, int rsiz, int framerate,
                                   bool apply_icc_,
                                   GRK_RATE_CONTROL_ALGORITHM rateControlAlgorithm, int numThreads, int deviceId,
                                   int duration, int kernelBuildOptions, int repeats, bool writePLT,
                                   bool writeTLM, bool verbose, bool sharedMemoryInterface);


#ifdef __cplusplus
}
#endif

#endif
