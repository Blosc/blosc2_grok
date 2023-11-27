import blosc2
import blosc2_grok
import argparse
from pathlib import Path
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
    # Add new axis corresponding to numcomps
    np_array = np_array[np.newaxis, ...]
    # np_array = np_array.astype(dtype=np.uint16)
    # print(np_array.dtype)

    # Make the array C-contiguous
    np_array = np_array.copy()

    blosc2.register_codec('grok', 160)

    # Set the parameters that will be used by the codec
    blosc2_grok.set_params_defaults(**kwargs)

    # Define the compression and decompression parameters. Disable the filters and the
    # splitmode, because these don't work with the codec.
    cparams = {
        'codec': 160,
        # 'nthreads': nthreads,
        'filters': [],
        'splitmode': blosc2.SplitMode.NEVER_SPLIT,
    }

    # Transform the numpy array to a blosc2 array. This is where compression happens, and
    # the HTJ2K codec is called.
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
        description='Compress the given input image using Blosc2 and OpenHTJ2K',
    )
    add_argument = parser.add_argument
    add_argument('inputfile')
    add_argument('outputfile')
    args = parser.parse_args()

    layer_rate = np.array([10], dtype=np.float64)
    # layer_rate[0] = 1
    layer_distortion = np.zeros(1, dtype=np.float64)
    layer_distortion[0] = 1
    kwargs = {'cod_format': blosc2_grok.GrkFileFmt.GRK_FMT_JP2,
              'verbose': True,
              # 'layer_rate': layer_rate, 'numlayers': 1,
              # 'layer_distortion': layer_distortion,
              # 'irreversible': True,
              }
    print(args)

    im = Image.open(args.inputfile)
    Path(args.outputfile).unlink(missing_ok=True)
    # print("mode: ", im.mode)
    # print("bands ", im.getbands())
    # print("size ", im.size)
    # print("info ", im.info)

    array = compress(im, args.outputfile, **kwargs)
