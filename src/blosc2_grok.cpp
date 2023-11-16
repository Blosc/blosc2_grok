/*********************************************************************
    Blosc - Blocked Shuffling and Compression Library

    Copyright (c) 2021  The Blosc Development Team <blosc@blosc.org>
    https://blosc.org
    License: GNU Affero General Public License v3.0 (see LICENSE.txt)
**********************************************************************/

#include <memory>

#include "grok.h"
#include "blosc2_grok.h"


grk_cparameters GRK_CPARAMETERS_DEFAULTS = {0};

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
    //grk_set_default_stream_params(streamParams);

    std::unique_ptr<uint8_t[]> data;
    size_t bufLen = (size_t)numComps * ((precision + 7) / 8) * dimX * dimY;
    data = std::make_unique<uint8_t[]>(bufLen);
    streamParams->buf = data.get();
    streamParams->buf_len = bufLen;

    // create image from input
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
        numComps, components, GRK_CLRSPC_GRAY, true);

    // fill in component data
    // see grok.h header for full details of image structure
    auto *ptr = (uint8_t*)input;
    for (uint16_t compno = 0; compno < image->numcomps; ++compno) {
        auto comp = image->comps + compno;
        auto compWidth = comp->w;
        auto compHeight = comp->h;
        auto compData = comp->data;
        if (!compData) {
            fprintf(stderr, "Image has null data for component %d\n", compno);
            goto beach;
        }
        // fill in component data, taking component stride into account
        auto srcData = new int32_t[compWidth * compHeight];
        for (uint32_t j = 0; j < compHeight; ++j) {
            for (uint32_t i = 0; i < compWidth; ++i) {
                memcpy(srcData + j * compWidth + i, ptr, typesize);
                ptr += typesize;
            }
        }

        auto srcPtr = srcData;
        for (uint32_t j = 0; j < compHeight; ++j) {
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
    size = (int)grk_compress(codec, nullptr);
    if (size == 0) {
        fprintf(stderr, "Failed to compress\n");
        goto beach;
    }
    if (size > output_len) {
        // Uncompressible data
        return 0;
    }
    memcpy(output, streamParams->buf, size);

beach:
    // cleanup
    delete[] components;
    grk_object_unref(codec);
    grk_object_unref(&image->obj);

    return size;
}

// Decompress a block
int blosc2_grok_decoder(const uint8_t *input, int32_t input_len, uint8_t *output, int32_t output_len,
                        uint8_t meta, blosc2_dparams *dparams, const void *chunk) {
    int size = -1;

    // initialize decompress parameters
    grk_decompress_parameters decompressParams;
    grk_decompress_set_default_params(&decompressParams);
    decompressParams.compressionLevel = GRK_DECOMPRESS_COMPRESSION_LEVEL_DEFAULT;
    decompressParams.verbose_ = true;

    grk_image *image = nullptr;
    grk_codec *codec = nullptr;

    // initialize decompressor
    grk_stream_params streamParams;
    grk_set_default_stream_params(&streamParams);
    streamParams.buf = (uint8_t *)input;
    streamParams.buf_len = input_len;
    codec = grk_decompress_init(&streamParams, &decompressParams.core);
    if (!codec) {
        fprintf(stderr, "Failed to set up decompressor\n");
        goto beach;
    }

    // read j2k header
    grk_header_info headerInfo;
    memset(&headerInfo, 0, sizeof(headerInfo));
    if (!grk_decompress_read_header(codec, &headerInfo)) {
        fprintf(stderr, "Failed to read the header\n");
        goto beach;
    }

    // retrieve image that will store uncompressed image data
    image = grk_decompress_get_composited_image(codec);
    if (!image) {
        fprintf(stderr, "Failed to retrieve image \n");
        goto beach;
    }

    // decompress all tiles
    if (!grk_decompress(codec, nullptr))
        goto beach;

    // see grok.h header for full details of image structure
    for (uint16_t compno = 0; compno < image->numcomps; ++compno) {
        auto comp = image->comps + compno;
        auto compWidth = comp->w;
        auto compHeight = comp->h;
        auto compData = comp->data;
        if (!compData) {
            fprintf(stderr, "Image has null data for component %d\n", compno);
            goto beach;
        }
        // copy data, taking component stride into account
        int itemsize = comp->prec / 8;
        memset(output, 0, output_len);
        auto copyPtr = output;
        for (uint32_t j = 0; j < compHeight; ++j) {
            auto compData = comp->data + compWidth * j;
            for (uint32_t i = 0; i < compWidth; ++i) {
                memcpy(copyPtr, compData, itemsize);
                copyPtr += itemsize;
                compData += 1;
            }
        }
    }

    size = output_len;
beach:
    // cleanup
    grk_object_unref(codec);

    return size;
}

void blosc2_grok_init(uint32_t nthreads, bool verbose) {
    // initialize library
    grk_initialize(nullptr, nthreads, verbose);
    // initialize grok defaults
    grk_compress_set_default_params(&GRK_CPARAMETERS_DEFAULTS);
}

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
                                    bool writeTLM, bool verbose, bool sharedMemoryInterface) {
    GRK_CPARAMETERS_DEFAULTS.tile_size_on = tile_size_on;
    GRK_CPARAMETERS_DEFAULTS.tx0 = tx0;
    GRK_CPARAMETERS_DEFAULTS.ty0 = ty0;
    GRK_CPARAMETERS_DEFAULTS.t_width = t_width;
    GRK_CPARAMETERS_DEFAULTS.t_height = t_height;

    GRK_CPARAMETERS_DEFAULTS.numlayers = numlayers;
    GRK_CPARAMETERS_DEFAULTS.allocationByRateDistoration = allocationByRateDistoration;

    for (int i = 0; i < GRK_MAX_LAYERS; ++i) {
        GRK_CPARAMETERS_DEFAULTS.layer_rate[i] = layer_rate[i];
        GRK_CPARAMETERS_DEFAULTS.layer_distortion[i] = layer_distortion[i];
    }
    GRK_CPARAMETERS_DEFAULTS.allocationByQuality = allocationByQuality;
    /*for (int i = 0; i < GRK_NUM_COMMENTS_SUPPORTED; ++i) {
        GRK_CPARAMETERS_DEFAULTS.comment[i] = comment[i]; // malloc & memcpy
        GRK_CPARAMETERS_DEFAULTS.comment_len[i] = comment_len[i];
        GRK_CPARAMETERS_DEFAULTS.is_binary_comment[i] = is_binary_comment[i];
    }
    GRK_CPARAMETERS_DEFAULTS.num_comments = num_comments;*/

    GRK_CPARAMETERS_DEFAULTS.csty = csty;
    GRK_CPARAMETERS_DEFAULTS.numgbits = numgbits;
    GRK_CPARAMETERS_DEFAULTS.prog_order = prog_order;
    for (int i = 0; i < GRK_J2K_MAXRLVLS; ++i) {
        // GRK_CPARAMETERS_DEFAULTS.progression[i] = progression[i];
        GRK_CPARAMETERS_DEFAULTS.prcw_init[i] = prcw_init[i];
        GRK_CPARAMETERS_DEFAULTS.prch_init[i] = prch_init[i];
    }
    GRK_CPARAMETERS_DEFAULTS.numpocs = numpocs;
    GRK_CPARAMETERS_DEFAULTS.numresolution = numresolution;

    GRK_CPARAMETERS_DEFAULTS.cblockw_init = cblockw_init;
    GRK_CPARAMETERS_DEFAULTS.cblockh_init = cblockh_init;
    GRK_CPARAMETERS_DEFAULTS.irreversible = irreversible;
    GRK_CPARAMETERS_DEFAULTS.roi_compno = roi_compno;
    GRK_CPARAMETERS_DEFAULTS.roi_shift = roi_shift;
    GRK_CPARAMETERS_DEFAULTS.res_spec = res_spec;

    GRK_CPARAMETERS_DEFAULTS.cblk_sty = cblk_sty;

    GRK_CPARAMETERS_DEFAULTS.image_offset_x0 = image_offset_x0;
    GRK_CPARAMETERS_DEFAULTS.image_offset_y0 = image_offset_y0;
    GRK_CPARAMETERS_DEFAULTS.subsampling_dx = subsampling_dx;
    GRK_CPARAMETERS_DEFAULTS.subsampling_dy = subsampling_dy;

    GRK_CPARAMETERS_DEFAULTS.decod_format = decod_format;
    GRK_CPARAMETERS_DEFAULTS.cod_format = cod_format;
    // GRK_CPARAMETERS_DEFAULTS.raw_cp = raw_cp;
    GRK_CPARAMETERS_DEFAULTS.enableTilePartGeneration = enableTilePartGeneration;
    GRK_CPARAMETERS_DEFAULTS.newTilePartProgressionDivider = newTilePartProgressionDivider;
    GRK_CPARAMETERS_DEFAULTS.mct = mct;

    // GRK_CPARAMETERS_DEFAULTS.mct_data = mct_data;
    GRK_CPARAMETERS_DEFAULTS.max_cs_size = max_cs_size;

    GRK_CPARAMETERS_DEFAULTS.max_comp_size = max_comp_size;
    GRK_CPARAMETERS_DEFAULTS.rsiz = rsiz;
    GRK_CPARAMETERS_DEFAULTS.framerate = framerate;

    for (int i = 0; i < 2; ++i) {
        GRK_CPARAMETERS_DEFAULTS.capture_resolution_from_file[i] = capture_resolution_from_file[i];
        GRK_CPARAMETERS_DEFAULTS.capture_resolution[i] = capture_resolution[i];
        GRK_CPARAMETERS_DEFAULTS.display_resolution[i] = display_resolution[i];
        GRK_CPARAMETERS_DEFAULTS.prcw_init[i] = prcw_init[i];
    }
    GRK_CPARAMETERS_DEFAULTS.write_capture_resolution_from_file = write_capture_resolution_from_file;
    GRK_CPARAMETERS_DEFAULTS.write_capture_resolution = write_capture_resolution;
    GRK_CPARAMETERS_DEFAULTS.write_display_resolution = write_display_resolution;
    GRK_CPARAMETERS_DEFAULTS.apply_icc_ = apply_icc_;
    GRK_CPARAMETERS_DEFAULTS.rateControlAlgorithm = rateControlAlgorithm;
    GRK_CPARAMETERS_DEFAULTS.numThreads = numThreads;
    GRK_CPARAMETERS_DEFAULTS.deviceId = deviceId;

    GRK_CPARAMETERS_DEFAULTS.duration = duration;
    GRK_CPARAMETERS_DEFAULTS.kernelBuildOptions = kernelBuildOptions;
    GRK_CPARAMETERS_DEFAULTS.repeats = repeats;
    GRK_CPARAMETERS_DEFAULTS.writePLT = writePLT;
    GRK_CPARAMETERS_DEFAULTS.writeTLM = writeTLM;

    GRK_CPARAMETERS_DEFAULTS.verbose = verbose;
    GRK_CPARAMETERS_DEFAULTS.sharedMemoryInterface = sharedMemoryInterface;
}

void blosc2_grok_destroy() {
    grk_deinitialize();
}
