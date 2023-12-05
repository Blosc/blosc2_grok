/*********************************************************************
Blosc - Blocked Shuffling and Compression Library

Copyright (c) 2021  The Blosc Development Team <blosc@blosc.org>
https://blosc.org
License: GNU Affero General Public License v3.0 (see LICENSE.txt)
**********************************************************************/


#ifndef PUBLIC_WRAPPER_H
#define PUBLIC_WRAPPER_H

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
#include "blosc2/codecs-registry.h"
#include "grok.h"


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

//BLOSC2_GROK_EXPORT codec_info info = {
//    .encoder=(char *)"blosc2_grok_encoder",
//    .decoder=(char *)"blosc2_grok_decoder"
//};

#if defined(_MSC_VER)
// Needed to export functions in Windows
BLOSC2_GROK_EXPORT void grk_initialize(const char* pluginPath, uint32_t numthreads, bool verbose);
BLOSC2_GROK_EXPORT void grk_deinitialize();
BLOSC2_GROK_EXPORT void grk_object_unref(grk_object* obj);
BLOSC2_GROK_EXPORT grk_image* grk_image_new(uint16_t numcmpts, grk_image_comp* cmptparms,
                                      GRK_COLOR_SPACE clrspc, bool alloc_data);
BLOSC2_GROK_EXPORT void grk_set_default_stream_params(grk_stream_params* params);
BLOSC2_GROK_EXPORT void grk_decompress_set_default_params(grk_decompress_parameters* parameters);
BLOSC2_GROK_EXPORT grk_codec* grk_decompress_init(grk_stream_params* stream_params,
                                            grk_decompress_core_params* core_params);
BLOSC2_GROK_EXPORT bool grk_decompress_read_header(grk_codec* codecWrapper, grk_header_info* header_info);
BLOSC2_GROK_EXPORT grk_image* grk_decompress_get_composited_image(grk_codec* codecWrapper);
BLOSC2_GROK_EXPORT bool grk_decompress(grk_codec* codecWrapper, grk_plugin_tile* tile);
BLOSC2_GROK_EXPORT void grk_compress_set_default_params(grk_cparameters* parameters);
BLOSC2_GROK_EXPORT grk_codec* grk_compress_init(grk_stream_params* stream_params,
                                          grk_cparameters* parameters, grk_image* p_image);
BLOSC2_GROK_EXPORT uint64_t grk_compress(grk_codec* codecWrapper, grk_plugin_tile* tile);
#endif


#ifdef __cplusplus
}
#endif

#endif
