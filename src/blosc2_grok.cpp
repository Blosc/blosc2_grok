/*********************************************************************
    Blosc - Blocked Shuffling and Compression Library

    Copyright (c) 2021  The Blosc Development Team <blosc@blosc.org>
    https://blosc.org
    License: BSD 3-Clause (see LICENSE.txt)

    See LICENSE.txt for details about copyright and rights to use.
**********************************************************************/

#include <memory>

#include "grok.h"
#include "blosc2_grok.h"


int blosc2_grok_encoder(
    const uint8_t *input,
    int32_t input_len,
    uint8_t *output,
    int32_t output_len,
    uint8_t meta,
    blosc2_cparams* cparams,
    const void* chunk
) {
    int size = -1;

    // Read blosc2 metadata
    uint8_t *content;
    int32_t content_len;
    BLOSC_ERROR(blosc2_meta_get((blosc2_schunk*)cparams->schunk, "b2nd",
                                &content, &content_len));

    int8_t ndim;
    int64_t shape[3];
    int32_t chunkshape[3];
    int32_t blockshape[3];
    char *dtype;
    int8_t dtype_format;
    BLOSC_ERROR(
        b2nd_deserialize_meta(content, content_len, &ndim,
                              shape, chunkshape, blockshape, &dtype, &dtype_format)
    );
    free(content);
    free(dtype);

    const uint32_t numComps = blockshape[0];
    const uint32_t dimX = blockshape[1];
    const uint32_t dimY = blockshape[2];
    const uint32_t typesize = ((blosc2_schunk*)cparams->schunk)->typesize;
    const uint32_t precision = typesize * 8;

    // initialize compress parameters
    grk_codec* codec = nullptr;
    blosc2_grok_params *codec_params = (blosc2_grok_params *)cparams->codec_params;
    grk_cparameters *compressParams = &codec_params->compressParams;
    grk_stream_params *streamParams = &codec_params->streamParams;
    grk_set_default_stream_params(streamParams);
    //WriteStreamInfo sinfo(&streamParams);

    std::unique_ptr<uint8_t[]> data;
    size_t bufLen = (size_t)numComps * ((precision + 7) / 8) * dimX * dimY;
    data = std::make_unique<uint8_t[]>(bufLen);
    streamParams->buf = data.get();
    streamParams->buf_len = bufLen;

    // create blank image
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
    grk_image* image = grk_image_new(
        numComps, components, GRK_CLRSPC_SRGB, true);

    // fill in component data
    // see grok.h header for full details of image structure

    auto *ptr = (uint8_t*)input;
    for (uint16_t compno = 0; compno < image->numcomps; ++compno) {
        auto comp = image->comps + compno;
        auto compWidth = comp->w;
        auto compHeight = comp->h;
        auto compData = comp->data;
        if(!compData) {
            fprintf(stderr, "Image has null data for component %d\n", compno);
            goto beach;
        }
        // fill in component data, taking component stride into account
        // in this example, we just zero out each component
        auto srcData = new int32_t[compWidth * compHeight];

        uint32_t len = compWidth * compHeight * typesize;
        memcpy(srcData, ptr, len);
        ptr += len;
        //memset(srcData, 0, compWidth * compHeight * sizeof(int32_t));

        auto srcPtr = srcData;
        for(uint32_t j = 0; j < compHeight; ++j) {
            memcpy(compData, srcPtr, compWidth * sizeof(int32_t));
            srcPtr += compWidth;
            compData += comp->stride;
        }
        delete[] srcData;
    }

    // initialize compressor
    codec = grk_compress_init(streamParams, compressParams, image);
    if (!codec) {
        fprintf(stderr, "Failed to initialize compressor\n");
        goto beach;
    }

    // compress
    printf("Abans de compress...    \n");
    size = (int)grk_compress(codec, nullptr);
    if (size == 0) {
        fprintf(stderr, "Failed to compress\n");
        goto beach;
    }

    printf("Compression succeeded: %d bytes used.\n", size);

beach:
    // cleanup
    delete[] components;
    grk_object_unref(codec);
    grk_object_unref(&image->obj);
    grk_deinitialize();

    return size;
}


void blosc2_grok_init(uint32_t nthreads, bool verbose) {
    // initialize library
    grk_initialize(nullptr, nthreads, verbose);
}

void blosc2_grok_destroy() {
    grk_deinitialize();
}
