/*********************************************************************
 * blosc2_grok: Grok (JPEG2000 codec) plugin for Blosc2
 *
 * Copyright (c) 2023  The Blosc Development Team <blosc@blosc.org>
 * https://blosc.org
 * License: GNU Affero General Public License v3.0 (see LICENSE.txt)
**********************************************************************/

#include <memory>

#include "blosc2_grok.h"
#include "blosc2_grok_public.h"

static grk_cparameters GRK_CPARAMETERS_DEFAULTS = {0};

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

    uint32_t dimX = blockshape[0];
    uint32_t dimY = blockshape[1];
    uint32_t numComps = 1;
    if (dimX == 1) {
        // Array formed by different images
        dimX = blockshape[1];
        dimY = blockshape[2];
    }
    else if (ndim == 3) {
        // Single image with more than 1 component
        numComps = blockshape[2];
    }

    const uint32_t typesize = ((blosc2_schunk*)cparams->schunk)->typesize;
    const uint32_t precision = 8 * typesize;
    // const uint32_t precision = 8 * typesize - 7;


    // initialize compress parameters
    grk_codec* codec = nullptr;
    blosc2_grok_params *codec_params = (blosc2_grok_params *)cparams->codec_params;
    grk_cparameters *compressParams;
    grk_stream_params *streamParams;

    if (codec_params == NULL) {
        compressParams = &GRK_CPARAMETERS_DEFAULTS;
        streamParams = (grk_stream_params *)malloc(sizeof(grk_stream_params));
        grk_set_default_stream_params(streamParams);
    } else {
        compressParams = &codec_params->compressParams;
        streamParams = &codec_params->streamParams;
    }

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
    grk_image* image;
    if (numComps == 1) {
        image = grk_image_new(
            numComps, components, GRK_CLRSPC_GRAY, true);

    } else {
        image = grk_image_new(
            numComps, components, GRK_CLRSPC_SRGB, true);
    }

    // fill in component data
    // see grok.h header for full details of image structure
    auto *ptr = (uint8_t*)input;
    uint64_t index = 0;
    for (uint16_t compno = 0; compno < image->numcomps; ++compno) {
        index = compno;
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
        memset(srcData, 0, compWidth * compHeight * sizeof(int32_t));
        for (uint32_t j = 0; j < compHeight; ++j) {
            for (uint32_t i = 0; i < compWidth; ++i) {
                memcpy(srcData + j * compWidth + i, &ptr[index * typesize], typesize);
                index += numComps;
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
        size = -1;
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
    if (codec_params == NULL) {
        free(streamParams);
    }

    return size;
}

int beach_decoder(grk_codec * codec, int rc) {
    // cleanup
    grk_object_unref(codec);
    return rc;
}

// Decompress a block
int blosc2_grok_decoder(const uint8_t *input, int32_t input_len, uint8_t *output, int32_t output_len,
                        uint8_t meta, blosc2_dparams *dparams, const void *chunk) {
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
        return beach_decoder(codec, BLOSC2_ERROR_FAILURE);
    }

    // read j2k header
    grk_header_info headerInfo;
    memset(&headerInfo, 0, sizeof(headerInfo));
    if (!grk_decompress_read_header(codec, &headerInfo)) {
        fprintf(stderr, "Failed to read the header\n");
        return beach_decoder(codec, BLOSC2_ERROR_FAILURE);
    }

    // retrieve image that will store uncompressed image data
    image = grk_decompress_get_composited_image(codec);
    if (!image) {
        fprintf(stderr, "Failed to retrieve image \n");
        return beach_decoder(codec, BLOSC2_ERROR_FAILURE);
    }

    // decompress all tiles
    if (!grk_decompress(codec, nullptr)){
        fprintf(stderr, "Error when decompressing image\n");
        return beach_decoder(codec, BLOSC2_ERROR_FAILURE);
    }

    // see grok.h header for full details of image structure
    memset(output, 0, output_len);
    auto copyPtr = output;
    uint64_t index = 0;
    for (uint16_t compno = 0; compno < image->numcomps; ++compno) {
        index = compno;
        auto comp = image->comps + compno;
        auto compWidth = comp->w;
        auto compHeight = comp->h;
        auto compData = comp->data;
        if (!compData) {
            fprintf(stderr, "Image has null data for component %d\n", compno);
            return beach_decoder(codec, BLOSC2_ERROR_FAILURE);
        }
        // copy data, taking component stride into account
        int itemsize =  (comp->prec / 8);
        // int itemsize =  ((comp->prec + 7) / 8);
        for (uint32_t j = 0; j < compHeight; ++j) {
            auto compData = comp->data + comp->stride * j;
            for (uint32_t i = 0; i < compWidth; ++i) {
                memcpy(&copyPtr[index * itemsize], compData, itemsize);
                compData += 1;
                index += image->numcomps;
            }
        }
    }

    return output_len;
}

void blosc2_grok_init(uint32_t nthreads, bool verbose) {
    // initialize library
    grk_initialize(nullptr, nthreads, verbose);
    // initialize grok defaults
    grk_compress_set_default_params(&GRK_CPARAMETERS_DEFAULTS);
}

void blosc2_grok_set_default_params(const int64_t *tile_size, const int64_t *tile_offset,
                                    int numlayers, char *quality_mode, double *quality_layers,
                                    int numgbits, char *progression,
                                    int num_resolutions, int64_t *codeblock_size,
                                    bool irreversible, int roi_compno, int roi_shift, const int64_t *precinct_size,
                                    const int64_t *offset,
                                    GRK_SUPPORTED_FILE_FMT decod_format,
                                    GRK_SUPPORTED_FILE_FMT cod_format, bool enableTilePartGeneration,
                                    int mct, int max_cs_size,
                                    int max_comp_size, int rsiz, int framerate,
                                    bool apply_icc_,
                                    GRK_RATE_CONTROL_ALGORITHM rateControlAlgorithm, int num_threads, int deviceId,
                                    int duration, int repeats,
                                    bool verbose) {
    if (tile_size[0] == 0 && tile_size[1] == 0) {
        GRK_CPARAMETERS_DEFAULTS.tile_size_on = false;
    } else {
        GRK_CPARAMETERS_DEFAULTS.tile_size_on = true;
    }
    GRK_CPARAMETERS_DEFAULTS.tx0 = tile_offset[0];
    GRK_CPARAMETERS_DEFAULTS.ty0 = tile_offset[1];
    GRK_CPARAMETERS_DEFAULTS.t_width = tile_size[0];
    GRK_CPARAMETERS_DEFAULTS.t_height = tile_size[1];

    GRK_CPARAMETERS_DEFAULTS.numlayers = numlayers;
    // Restore default values
    GRK_CPARAMETERS_DEFAULTS.allocationByRateDistoration = false;
    GRK_CPARAMETERS_DEFAULTS.allocationByQuality = false;
    if (quality_mode != nullptr) {
        if (strcmp(quality_mode, "rates") == 0) {
            GRK_CPARAMETERS_DEFAULTS.allocationByRateDistoration = true;
            for (int i = 0; i < numlayers; ++i) {
                GRK_CPARAMETERS_DEFAULTS.layer_rate[i] = quality_layers[i];
            }
        } else if (strcmp(quality_mode, "dB") == 0) {
            GRK_CPARAMETERS_DEFAULTS.allocationByQuality = true;
            for (int i = 0; i < numlayers; ++i) {
                GRK_CPARAMETERS_DEFAULTS.layer_distortion[i] = quality_layers[i];
            }
        }
    }

    /*for (int i = 0; i < GRK_NUM_COMMENTS_SUPPORTED; ++i) {
        GRK_CPARAMETERS_DEFAULTS.comment[i] = comment[i]; // malloc & memcpy
        GRK_CPARAMETERS_DEFAULTS.comment_len[i] = comment_len[i];
        GRK_CPARAMETERS_DEFAULTS.is_binary_comment[i] = is_binary_comment[i];
    }
    GRK_CPARAMETERS_DEFAULTS.num_comments = num_comments;*/

    // GRK_CPARAMETERS_DEFAULTS.csty = csty;
    GRK_CPARAMETERS_DEFAULTS.numgbits = numgbits;
    if (strcmp(progression, "LRCP") == 0) {
        GRK_CPARAMETERS_DEFAULTS.prog_order = GRK_LRCP;
    } else if (strcmp(progression, "RLCP") == 0) {
        GRK_CPARAMETERS_DEFAULTS.prog_order = GRK_RLCP;
    } else if (strcmp(progression, "RPCL") == 0) {
        GRK_CPARAMETERS_DEFAULTS.prog_order = GRK_RPCL;
    } else if (strcmp(progression, "PCRL") == 0) {
        GRK_CPARAMETERS_DEFAULTS.prog_order = GRK_PCRL;
    } else if (strcmp(progression, "CPRL") == 0) {
        GRK_CPARAMETERS_DEFAULTS.prog_order = GRK_CPRL;
    }

    //for (int i = 0; i < res_spec; ++i) {
        // GRK_CPARAMETERS_DEFAULTS.progression[i] = progression[i];
    // }
    if (precinct_size[0] != 0 && precinct_size[1] != 0) {
        GRK_CPARAMETERS_DEFAULTS.res_spec = 1; // grok can support more than one, but PIL not.
    } else {
        GRK_CPARAMETERS_DEFAULTS.res_spec = 0;
    }
    GRK_CPARAMETERS_DEFAULTS.prcw_init[0] = precinct_size[0];
    GRK_CPARAMETERS_DEFAULTS.prch_init[0] = precinct_size[1];
    // GRK_CPARAMETERS_DEFAULTS.numpocs = numpocs; only one prog supported
    GRK_CPARAMETERS_DEFAULTS.numresolution = num_resolutions;

    GRK_CPARAMETERS_DEFAULTS.cblockw_init = codeblock_size[0];
    GRK_CPARAMETERS_DEFAULTS.cblockh_init = codeblock_size[1];


    GRK_CPARAMETERS_DEFAULTS.irreversible = irreversible;
    GRK_CPARAMETERS_DEFAULTS.roi_compno = roi_compno;
    GRK_CPARAMETERS_DEFAULTS.roi_shift = roi_shift;

    // GRK_CPARAMETERS_DEFAULTS.cblk_sty = codeblock_style;

    GRK_CPARAMETERS_DEFAULTS.image_offset_x0 = offset[0];
    GRK_CPARAMETERS_DEFAULTS.image_offset_y0 = offset[1];
    // GRK_CPARAMETERS_DEFAULTS.subsampling_dx = subsampling_dx;
    // GRK_CPARAMETERS_DEFAULTS.subsampling_dy = subsampling_dy;

    GRK_CPARAMETERS_DEFAULTS.decod_format = decod_format;
    GRK_CPARAMETERS_DEFAULTS.cod_format = cod_format;
    // GRK_CPARAMETERS_DEFAULTS.raw_cp = raw_cp;
    GRK_CPARAMETERS_DEFAULTS.enableTilePartGeneration = enableTilePartGeneration;
    // GRK_CPARAMETERS_DEFAULTS.newTilePartProgressionDivider = newTilePartProgressionDivider;
    GRK_CPARAMETERS_DEFAULTS.mct = mct;

    // GRK_CPARAMETERS_DEFAULTS.mct_data = mct_data;
    GRK_CPARAMETERS_DEFAULTS.max_cs_size = max_cs_size;

    GRK_CPARAMETERS_DEFAULTS.max_comp_size = max_comp_size;
    GRK_CPARAMETERS_DEFAULTS.rsiz = rsiz;
    GRK_CPARAMETERS_DEFAULTS.framerate = framerate;

    /*for (int i = 0; i < 2; ++i) {
        GRK_CPARAMETERS_DEFAULTS.capture_resolution_from_file[i] = capture_resolution_from_file[i];
        GRK_CPARAMETERS_DEFAULTS.capture_resolution[i] = capture_resolution[i];
        GRK_CPARAMETERS_DEFAULTS.display_resolution[i] = display_resolution[i];
    }
    GRK_CPARAMETERS_DEFAULTS.write_capture_resolution_from_file = write_capture_resolution_from_file;
    GRK_CPARAMETERS_DEFAULTS.write_capture_resolution = write_capture_resolution;
    GRK_CPARAMETERS_DEFAULTS.write_display_resolution = write_display_resolution;*/
    GRK_CPARAMETERS_DEFAULTS.apply_icc_ = apply_icc_;
    GRK_CPARAMETERS_DEFAULTS.rateControlAlgorithm = rateControlAlgorithm;
    GRK_CPARAMETERS_DEFAULTS.numThreads = num_threads;
    GRK_CPARAMETERS_DEFAULTS.deviceId = deviceId;

    GRK_CPARAMETERS_DEFAULTS.duration = duration;
    // GRK_CPARAMETERS_DEFAULTS.kernelBuildOptions = kernelBuildOptions;
    GRK_CPARAMETERS_DEFAULTS.repeats = repeats;
    // GRK_CPARAMETERS_DEFAULTS.writePLT = writePLT;
    // GRK_CPARAMETERS_DEFAULTS.writeTLM = writeTLM;

    GRK_CPARAMETERS_DEFAULTS.verbose = verbose;
    // GRK_CPARAMETERS_DEFAULTS.sharedMemoryInterface = sharedMemoryInterface;

    // Initialize threads and verbose
    grk_initialize(nullptr, GRK_CPARAMETERS_DEFAULTS.numThreads, GRK_CPARAMETERS_DEFAULTS.verbose);
}

void blosc2_grok_destroy() {
    grk_deinitialize();
}
