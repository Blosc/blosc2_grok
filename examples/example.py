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

    Other keyword arguments are passed to blosc2_openhtj2k.set_params_defaults(...)
    """
    # Convert the image to a numpy array
    np_array = np.asarray(im)
    # np_array = np.transpose(np_array)
    np_array = np_array[np.newaxis, ...]
    print(np_array.dtype)
    # np_array = np_array.astype(dtype=np.uint16)
    print(np_array.shape)
    # print(np_array.dtype)

    # Transpose the array so the channel (color) comes first
    # Change from (height, width, channel) to (channel, width, height)
    # np_array = np.transpose(np_array, (2, 1, 0))

    # Make the array C-contiguous
    np_array = np_array.copy()

    # The library expects 4 bytes per color (xx 00 00 00), so change the type
    # np_array = np_array.astype('uint32')

    blosc2.register_codec('grok', 160)

    # Set the parameters that will be used by the codec
    # blosc2_grok.set_params_defaults(**kwargs)

    # Multithreading is not supported, so we must set the number of threads to 1
    nthreads = 1

    # Define the compression and decompression paramaters. Disable the filters and the
    # splitmode, because these don't work with the codec.
    cparams = {
        'codec': 160,
        'nthreads': nthreads,
        'filters': [],
        'splitmode': blosc2.SplitMode.NEVER_SPLIT,
    }
    dparams = {'nthreads': nthreads}

    # Transform the numpy array to a blosc2 array. This is where compression happens, and
    # the HTJ2K codec is called.
    bl_array = blosc2.asarray(
        np_array,
        chunks=np_array.shape,
        blocks=np_array.shape,
        cparams=cparams,
        dparams=dparams,
        # urlpath=urlpath,
    )

    # Print information about the array, see the compression ratio (cratio)
    print(bl_array.info)

    np.testing.assert_array_equal(bl_array[:], np_array)

    return bl_array


if __name__ == '__main__':
    # parser = argparse.ArgumentParser(
    #     description='Compress the given input image using Blosc2 and OpenHTJ2K',
    # )
    # add_argument = parser.add_argument
    # add_argument('inputfile')
    # add_argument('outputfile')
    # add_argument('--transformation', type=int, help='0:lossy 1:lossless (default is 1)')
    # add_argument('--qfactor', type=int, help='Quality factor: 0-100')
    # #add_argument('--isJPH', action='store_true', help='')
    # add_argument('--color-space', type=int, help='0:RGB 1:YCC')
    # add_argument('--nthreads', type=int, help='Number of threads')
    # add_argument('--blkwidth', type=int, help='Precinct width (default: 4)')
    # add_argument('--blkheight', type=int, help='Precinct height (default: 4)')
    # args = parser.parse_args()
    #
    # kwargs = {
    #     k: v for k, v in args._get_kwargs()
    #     if k in blosc2_openhtj2k.params_defaults and v is not None
    # }

    layer_rate = np.array([10], dtype=np.float64)
    # layer_rate[0] = 1
    layer_distortion = np.zeros(1, dtype=np.float64)
    layer_distortion[0] = 1
    args = {'cod_format': blosc2_grok.GrkFileFmt.GRK_FMT_JP2,
            'verbose': True,
            # 'decod_format': blosc2_grok.GrkFileFmt.GRK_FMT_TIF,
            # 'layer_rate': layer_rate, 'numlayers': 1,
            # 'layer_distortion': layer_distortion,
            # 'irreversible': True,
            }
    print(args)

    infile = "./MI04_020751.tif"
    # infile = "./pexels.jpg"
    # infile = "./lion1.png"
    outfile = "arr.b2nd"
    im = Image.open(infile)
    Path(outfile).unlink(missing_ok=True)
    print("mode: ", im.mode)
    print("bands ", im.getbands())
    print("size ", im.size)
    print("info ", im.info)
    # print(im.show())

    compress(im, outfile, **args)
