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
import time


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
        'codec': 160,
        'filters': [],
        'splitmode': blosc2.SplitMode.NEVER_SPLIT,
    }

    # Transform the numpy array to a blosc2 array. This is where compression happens, and
    # the HTJ2K codec is called.
    start = time.time()
    bl_array = blosc2.asarray(
        np_array,
        chunks=np_array.shape,
        blocks=np_array.shape,
        cparams=cparams,
        urlpath=urlpath,
        mode="w",
    )
    stop = time.time()
    print("Time for compressing: ", stop - start)
    # Print information about the array, see the compression ratio (cratio)
    print(bl_array.info)

    if kwargs.get('quality_mode', None) is None:
        np.testing.assert_array_equal(bl_array[...], np_array)
    else:
        _ = bl_array[...]

    return bl_array[...]


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Compress the given input image using Blosc2 and grok',
    )
    add_argument = parser.add_argument
    add_argument('inputfile')
    add_argument('-o', '--outputfile', type=str, help='File name from decompressed image')
    args = parser.parse_args()
    im = Image.open(args.inputfile)

    # Register codec locally for now
    blosc2.register_codec('grok', 160)
    # Make a first run to initialize everything
    kwargs = {}
    _ = compress(im, "lossless.b2nd", **kwargs)

    print("Performing lossless compression with 1 thread ...")
    kwargs['num_threads'] = 1
    _ = compress(im, "lossless.b2nd", **kwargs)

    print("Performing lossless compression with default threads ...")
    kwargs['num_threads'] = 0
    _ = compress(im, "lossless.b2nd", **kwargs)

    print("Performing rates compression ...")
    kwargs['quality_mode'] = "rates"
    kwargs['quality_layers'] = np.array([10], dtype=np.float64)
    arr = compress(im, "rates.b2nd", **kwargs)

    if args.outputfile is not None:
        im = Image.fromarray(arr)
        im.save(args.outputfile + 'rates.png')

    print("Performing dB compression ...")
    kwargs['quality_mode'] = "dB"
    kwargs['quality_layers'] = np.array([45], dtype=np.float64)
    arr = compress(im, "dB.b2nd", **kwargs)
    if args.outputfile is not None:
        im = Image.fromarray(arr)
        im.save(args.outputfile + 'dB.png')
