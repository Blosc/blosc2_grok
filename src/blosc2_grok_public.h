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

BLOSC2_GROK_EXPORT codec_info info = {
    .encoder=(char *)"blosc2_grok_encoder",
    .decoder=(char *)"blosc2_grok_decoder"
};

#ifdef __cplusplus
}
#endif

#endif
