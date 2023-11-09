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
    // Read blosc2 metadata
    uint8_t *content;
    int32_t content_len;
    BLOSC_ERROR(blosc2_meta_get((blosc2_schunk*)cparams->schunk, "b2nd", &content, &content_len));

    int8_t ndim;
    int64_t shape[3];
    int32_t chunkshape[3];
    int32_t blockshape[3];
    char *dtype;
    int8_t dtype_format;
    BLOSC_ERROR(
        b2nd_deserialize_meta(content, content_len, &ndim, shape, chunkshape, blockshape, &dtype, &dtype_format)
    );
    free(content);
    free(dtype);

    const uint32_t numComps = blockshape[0];
    const uint32_t dimX = blockshape[1];
    const uint32_t dimY = blockshape[2];
    const uint32_t precision = ((blosc2_schunk*)cparams->schunk)->typesize * 8;

    // initialize compress parameters
    grk_cparameters compressParams;
    grk_compress_set_default_params(&compressParams);
    compressParams.cod_format = GRK_FMT_JP2;
    compressParams.verbose = true;

    grk_codec* codec = nullptr;
    grk_image* image = nullptr;
    grk_image_comp* components = nullptr;

    grk_stream_params streamParams;
    grk_set_default_stream_params(&streamParams);
//  WriteStreamInfo sinfo(&streamParams);

    std::unique_ptr<uint8_t[]> data;
    size_t bufLen = (size_t)numComps * ((precision + 7) / 8) * dimX * dimY;
    data = std::make_unique<uint8_t[]>(bufLen);
    streamParams.buf = data.get();
    streamParams.buf_len = bufLen;

    // create blank image
    components = new grk_image_comp[numComps];
    for(uint32_t i = 0; i < numComps; ++i) {
        auto c = components + i;
        c->w = dimX;
        c->h = dimY;
        c->dx = 1;
        c->dy = 1;
        c->prec = precision;
        c->sgnd = false;
    }
    image = grk_image_new(numComps, components, GRK_CLRSPC_SRGB, true);


    const int size = 0;
    return size;
}


void blosc2_grok_init() {
    // initialize library
    grk_initialize(nullptr, 0, false);
}
