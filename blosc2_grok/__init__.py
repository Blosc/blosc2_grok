# Blosc - Blocked Shuffling and Compression Library
#
# Copyright (C) 2023  The Blosc Developers <blosc@blosc.org>
# https://blosc.org
# License: BSD 3-Clause (see LICENSE.txt)
#
# See LICENSE.txt for details about copyright and rights to use.

import ctypes
import os
import platform
from enum import Enum
from pathlib import Path
import atexit


class GrkProgOrder(Enum):
    """
    Available grok progression orders.
    `L` : layer
    `R` : resolution
    `C` : component
    `P` : precinct
    """

    LRCP = 0
    RLCP = 1
    RPCL = 2
    PCRL = 3
    CPRL = 4


class GrkFileFmt(Enum):
    """
    Supported file formats in grok.
    """

    GRK_FMT_UNK = 0
    GRK_FMT_J2K = 1
    GRK_FMT_JP2 = 2
    GRK_FMT_PXM = 3
    GRK_FMT_PGX = 4
    GRK_FMT_PAM = 5
    GRK_FMT_BMP = 6
    GRK_FMT_TIF = 7
    GRK_FMT_RAW = 8  # Big Endian
    GRK_FMT_PNG = 9
    GRK_FMT_RAWL = 10  # Little Endian
    GRK_FMT_JPG = 11


class GrkRateControl(Enum):
    """
    Available grok rate control algorithms.
    """

    BISECT = 0  # bisect with all truncation points
    PCRD_OPT = 1  # bisect with only feasible truncation points


def get_libpath():
    system = platform.system()
    if system in ["Linux", "Darwin"]:
        libname = "libblosc2_grok.so"
    elif system == "Windows":
        libname = "blosc2_grok.dll"
    else:
        raise RuntimeError("Unsupported system: ", system)
    return os.path.abspath(Path(__file__).parent / libname)


libpath = get_libpath()
lib = ctypes.cdll.LoadLibrary(libpath)


def print_libpath():
    print(libpath, end="")


# Initialize grok
lib.blosc2_grok_init(0, False)

# Deinitialize grok when exiting
@atexit.register
def destroy():
    lib.blosc2_grok_destroy()


# TODO: change these for real defaults
params_defaults = {
    'tile_size_on': False,
    'tx0': 0,
    'ty0': 0,
    't_width': 0,
    't_height': 0,
    'numlayers': 0,
    'allocationByRateDistoration': False,
    'layer_rate': [0] * 100,
    'allocationByQuality': False,
    'layer_distortion': [0] * 100,
    'csty': 0,
    'numgbits': 0,
    'prog_order': GrkProgOrder.LRCP,
    'numpocs': 0,
    'numresolution': 0,
    'cblockw_init': 0,
    'cblockh_init': 0,
    'cblk_sty': 0,
    'irreversible': False,
    'roi_compno': 0,
    'roi_shift': 0,
    'res_spec': 0,
    'prcw_init': [0] * 33,
    'prch_init': [0] * 33,
    'image_offset_x0': 0,
    'image_offset_y0': 0,
    'subsampling_dx': 0,
    'subsampling_dy': 0,
    'decod_format': GrkFileFmt.GRK_FMT_UNK,
    'cod_format': GrkFileFmt.GRK_FMT_UNK,
    'enableTilePartGeneration': False,
    'newTilePartProgressionDivider': 0,
    'mct': 0,
    'max_cs_size': 0,
    'max_comp_size': 0,
    'rsiz': 0,
    'framerate': 0,
    'write_capture_resolution_from_file': False,
    'capture_resolution': [0] * 2,
    'write_display_resolution': False,
    'display_resolution': [0] * 2,
    'apply_icc_': False,
    'rateControlAlgorithm': GrkRateControl.PCRD_OPT,
    'numThreads': 0,
    'deviceId': 0,
    'duration': 0,
    'kernelBuildOptions': 0,
    'repeats': 0,
    'writePLT': False,
    'writeTLM': False,
    'verbose': False,
    'sharedMemoryInterface': False,
}



#
# def set_params_defaults(**kwargs):
#     # Check arguments
#     not_supported = [k for k in kwargs.keys() if k not in params_defaults]
#     if not_supported != []:
#         raise ValueError(f"The next params are not supported: {not_supported}")
#
#     # Prepare arguments
#     params = params_defaults.copy()
#     params.update(kwargs)
#     args = params.values()
#     args = list(args)
#     args[17] = ctypes.c_double(args[17])
#
#     lib.set_params_defaults(*args)


if __name__ == "__main__":
    print_libpath()
