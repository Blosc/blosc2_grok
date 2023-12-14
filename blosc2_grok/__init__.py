##############################################################################
# blosc2_grok: Grok (JPEG2000 codec) plugin for Blosc2
#
# Copyright (c) 2023  The Blosc Development Team <blosc@blosc.org>
# https://blosc.org
# License: GNU Affero General Public License v3.0 (see LICENSE.txt)
##############################################################################

import ctypes
import os
import platform
from enum import Enum
from pathlib import Path
import atexit
import numpy as np

__version__ = "0.1.0"


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


class GrkProfile(Enum):
    """
    Available grok profiles.
    """

    GRK_PROFILE_NONE = 0x0000
    GRK_PROFILE_0 = 0x0001
    GRK_PROFILE_1 = 0x0002
    GRK_PROFILE_CINEMA_2K = 0x0003
    GRK_PROFILE_CINEMA_4K = 0x0004
    GRK_PROFILE_CINEMA_S2K = 0x0005
    GRK_PROFILE_CINEMA_S4K = 0x0006
    GRK_PROFILE_CINEMA_LTS = 0x0007
    GRK_PROFILE_BC_SINGLE = 0x0100  # Has to be combined with the target level (3-0 LSB, value between 0 and 11)
    GRK_PROFILE_BC_MULTI = 0x0200  # Has to be combined with the target level (3-0 LSB, value between 0 and 11)
    GRK_PROFILE_BC_MULTI_R = 0x0300  # Has to be combined with the target level (3-0 LSB, value between 0 and 11)
    GRK_PROFILE_BC_MASK = 0x030F  # Has to be combined with the target level (3-0 LSB, value between 0 and 11)
    GRK_PROFILE_IMF_2K = 0x0400  # Has to be combined with the target main-level (3-0 LSB, value between 0 and 11)
    # and sub-level (7-4 LSB, value between 0 and 9)
    GRK_PROFILE_IMF_4K = 0x0500  # Has to be combined with the target main-level (3-0 LSB, value between 0 and 11)
    # and sub-level (7-4 LSB, value between 0 and 9)
    GRK_PROFILE_IMF_8K = 0x0600  # Has to be combined with the target main-level (3-0 LSB, value between 0 and 11)
    # and sub-level (7-4 LSB, value between 0 and 9)
    GRK_PROFILE_IMF_2K_R = 0x0700  # Has to be combined with the target main-level (3-0 LSB, value between 0 and 11)
    # and sub-level (7-4 LSB, value between 0 and 9)
    GRK_PROFILE_IMF_4K_R = 0x0800  # Has to be combined with the target main-level (3-0 LSB, value between 0 and 11)
    # and sub-level (7-4 LSB, value between 0 and 9)
    GRK_PROFILE_IMF_8K_R = 0x0900  # Has to be combined with the target main-level (3-0 LSB, value between 0 and 11)
    # and sub-level (7-4 LSB, value between 0 and 9)
    GRK_PROFILE_MASK = 0x0FFF
    GRK_PROFILE_PART2 = 0x8000  # Must be combined with extensions
    GRK_PROFILE_PART2_EXTENSIONS_MASK = 0x3FFF


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


params_defaults = {
    'tile_size': (0, 0),
    'tile_offset': (0, 0),
    # 'numlayers': 0, # blosc2_grok C func set_params will still receive this param
    'quality_mode': None,
    'quality_layers': np.zeros(0, dtype=np.float64),
    'numgbits': 2,
    'progression': "LRCP",
    'num_resolutions': 6,
    'codeblock_size': (64, 64),
    'irreversible': False,
    # 10 - 19
    'roi_compno': -1,
    'roi_shift': 0,
    'precinct_size': (0, 0),
    'offset': (0, 0),
    'decod_format': GrkFileFmt.GRK_FMT_UNK,
    'cod_format': GrkFileFmt.GRK_FMT_UNK,
    'enableTilePartGeneration': False,
    'mct': 0,
    'max_cs_size': 0,
    'max_comp_size': 0,
    # 20 - 28
    'rsiz': GrkProfile.GRK_PROFILE_NONE,
    'framerate': 0,
    'apply_icc_': False,
    'rateControlAlgorithm': GrkRateControl.BISECT,
    'num_threads': 0,
    'deviceId': 0,
    'duration': 0,
    'repeats': 1,
    'verbose': False,
}


def set_params_defaults(**kwargs):
    # Check arguments
    not_supported = [k for k in kwargs.keys() if k not in params_defaults]
    if not_supported != []:
        raise ValueError(f"The next params are not supported: {not_supported}")

    # Prepare arguments
    params = params_defaults.copy()
    params.update(kwargs)
    args = params.values()
    args = list(args)

    # Get number of layers
    args.insert(2, 0)
    if args[3] is not None:
        args[3] = args[3].encode('utf-8')
        args[2] = args[4].shape[0]

    args[6] = args[6].encode('utf-8')

    # Convert tuples to desired NumPy arrays
    args[0] = np.array(args[0], dtype=np.int64)
    args[1] = np.array(args[1], dtype=np.int64)
    args[8] = np.array(args[8], dtype=np.int64)
    args[12] = np.array(args[2], dtype=np.int64)
    args[13] = np.array(args[13], dtype=np.int64)

    # Get value of enumerate
    args[14] = args[14].value
    args[15] = args[15].value
    args[20] = args[20].value
    args[23] = args[23].value

    lib.blosc2_grok_set_default_params.argtypes = ([np.ctypeslib.ndpointer(dtype=np.int64)] * 2 +
                                                   [ctypes.c_int] + [ctypes.c_char_p] + [np.ctypeslib.ndpointer(dtype=np.float64)] +
                                                   [ctypes.c_int] + [ctypes.c_char_p] +
                                                   [ctypes.c_int] + [np.ctypeslib.ndpointer(dtype=np.int64)] +
                                                   [ctypes.c_bool] + [ctypes.c_int] * 2 + [np.ctypeslib.ndpointer(dtype=np.int64)] +
                                                   [np.ctypeslib.ndpointer(dtype=np.int64)] +
                                                   [ctypes.c_int] +
                                                   [ctypes.c_int] + [ctypes.c_bool] +
                                                   [ctypes.c_int] * 5 + [ctypes.c_bool] +
                                                   [ctypes.c_int] * 5 + [ctypes.c_bool])
    lib.blosc2_grok_set_default_params(*args)


if __name__ == "__main__":
    print_libpath()
