/*********************************************************************
Blosc - Blocked Shuffling and Compression Library

Copyright (c) 2021  The Blosc Development Team <blosc@blosc.org>
https://blosc.org
License: GNU Affero General Public License v3.0 (see LICENSE.txt)
**********************************************************************/


#ifndef WRAPPER_H
#define WRAPPER_H

#if defined(_MSC_VER)
#define BLOSC2_GROK_EXPORT __declspec(dllexport)
#elif (defined(__GNUC__) && __GNUC__ >= 4) || defined(__clang__)
#if defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
#define BLOSC2_GROK_EXPORT __attribute__((dllexport))
#else
#define BLOSC2_GROK_EXPORT __attribute__((visibility("default")))
#endif  /* defined(_WIN32) || defined(__CYGWIN__) */
#else
#error Cannot determine how to define BLOSC2_GROK_EXPORT for this compiler.
#endif

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

#include "blosc2.h"
#include "b2nd.h"
#include "blosc2/codecs-registry.h"

typedef struct {
    grk_cparameters compressParams;
    grk_stream_params streamParams;
} blosc2_grok_params;

BLOSC2_GROK_EXPORT int blosc2_grok_encoder(
    const uint8_t *input,
    int32_t input_len,
    uint8_t *output,
    int32_t output_len,
    uint8_t meta,
    blosc2_cparams* cparams,
    const void* chunk
);

BLOSC2_GROK_EXPORT int blosc2_grok_decoder(const uint8_t *input, int32_t input_len, uint8_t *output, int32_t output_len,
                        uint8_t meta, blosc2_dparams *dparams, const void *chunk);

void blosc2_grok_init(uint32_t nthreads, bool verbose);
void blosc2_grok_destroy();

void blosc2_grok_set_default_params(bool tile_size_on, uint32_t tx0, uint32_t ty0, uint32_t t_width, uint32_t t_height,
                                    uint16_t numlayers, bool allocationByRateDistoration,
                                    double layer_rate[GRK_MAX_LAYERS], bool allocationByQuality,
                                    double layer_distortion[GRK_MAX_LAYERS],
                                    uint8_t csty, uint8_t numgbits, GRK_PROG_ORDER prog_order,
                                    uint32_t numpocs,
                                    uint8_t numresolution, uint32_t cblockw_init, uint32_t cblockh_init, uint8_t cblk_sty,
                                    bool irreversible, int32_t roi_compno, uint32_t roi_shift, uint32_t res_spec,
                                    uint32_t prcw_init[GRK_J2K_MAXRLVLS], uint32_t prch_init[GRK_J2K_MAXRLVLS],
                                    uint32_t image_offset_x0, uint32_t image_offset_y0, uint8_t subsampling_dx,
                                    uint8_t subsampling_dy, GRK_SUPPORTED_FILE_FMT decod_format,
                                    GRK_SUPPORTED_FILE_FMT cod_format, bool enableTilePartGeneration,
                                    uint8_t newTilePartProgressionDivider, uint8_t mct, uint64_t max_cs_size,
                                    uint64_t max_comp_size, uint16_t rsiz, uint16_t framerate,
                                    bool write_capture_resolution_from_file, double capture_resolution_from_file[2],
                                    bool write_capture_resolution, double capture_resolution[2],
                                    bool write_display_resolution, double display_resolution[2], bool apply_icc_,
                                    GRK_RATE_CONTROL_ALGORITHM rateControlAlgorithm, uint32_t numThreads, int32_t deviceId,
                                    uint32_t duration, uint32_t kernelBuildOptions, uint32_t repeats, bool writePLT,
                                    bool writeTLM, bool verbose, bool sharedMemoryInterface);

BLOSC2_GROK_EXPORT codec_info info = {
    .encoder=(char *)"blosc2_grok_encoder",
    .decoder=(char *)"blosc2_grok_decoder"
};

#ifdef __cplusplus
}
#endif

#endif
