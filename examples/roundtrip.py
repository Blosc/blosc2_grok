##############################################################################
# blosc2_grok: Grok (JPEG2000 codec) plugin for Blosc2
#
# Copyright (c) 2023  The Blosc Development Team <blosc@blosc.org>
# https://blosc.org
# License: GNU Affero General Public License v3.0 (see LICENSE.txt)
##############################################################################

import blosc2
import blosc2_grok
import argparse
import numpy as np
from PIL import Image


def compress(im, urlpath=None, **kwargs):
    """
    This function gets a PIL image and returns a Blosc2 array.

    If the optional argument urlpath is given, the Blosc2 array will be saved to that
    location.

    Other keyword arguments are passed to blosc2_grok.set_params_defaults(...)
    """
    # Convert the image to a numpy array
    np_array = np.asarray(im)

    # Set the parameters that will be used by the codec
    blosc2_grok.set_params_defaults(**kwargs)

    # Define the compression and decompression parameters. Disable the filters and the
    # splitmode, because these don't work with the codec.
    cparams = {
        'codec': blosc2.Codec.GROK,
        'filters': [],
        'splitmode': blosc2.SplitMode.NEVER_SPLIT,
    }

    # Transform the numpy array to a blosc2 array. This is where compression happens, and
    # the grok codec is called.
    bl_array = blosc2.asarray(
        np_array,
        chunks=np_array.shape,
        blocks=np_array.shape,
        cparams=cparams,
        urlpath=urlpath,
    )

    # Print information about the array, see the compression ratio (cratio)
    print(bl_array.info)

    np.testing.assert_array_equal(bl_array[:], np_array)

    return bl_array


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Compress the given input image using Blosc2 and grok',
    )
    add_argument = parser.add_argument
    add_argument('inputfile')
    args = parser.parse_args()

    quality_mode = 'rates'
    # quality_mode = 'dB'
    quality_layers = np.array([5], dtype=np.float64)
    kwargs = {'cod_format': blosc2_grok.GrkFileFmt.GRK_FMT_JP2,
              'verbose': True,
              # 'quality_mode': quality_mode,
              # 'quality_layers': quality_layers,
              }
    print(kwargs)

    im = Image.open(args.inputfile)

    array = compress(im, **kwargs)
