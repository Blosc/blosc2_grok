##############################################################################
# blosc2_grok: Grok (JPEG2000 codec) plugin for Blosc2
#
# Copyright (c) 2023  The Blosc Development Team <blosc@blosc.org>
# https://blosc.org
# License: GNU Affero General Public License v3.0 (see LICENSE.txt)
##############################################################################

"""
Benchmark for compressing a dataset of images with the grok codec.

This uses the second partition in Blosc2 for encoding the JPEG2000 data.

Data can be downloaded from: http://www.silx.org/pub/nabu/data/compression/lung_raw_2000-2100.h5
"""

import h5py
import blosc2
import blosc2_grok
import numpy as np
from skimage.metrics import structural_similarity as ssim
from tqdm import tqdm
from time import time


if __name__ == '__main__':
    # Define the compression and decompression parameters. Disable the filters and the
    # splitmode, because these don't work with the codec.
    cparams = {
        'codec': blosc2.Codec.GROK,
        'nthreads': 4,
        'filters': [],
        'splitmode': blosc2.SplitMode.NEVER_SPLIT,
    }
    cparams_blosc2 = blosc2.cparams_dflts.copy()
    # cparams_blosc2['clevel'] = 9
    # cparams_blosc2['nthreads'] = 4
    # cparams_blosc2['filters'] = [blosc2.Filter.BITSHUFFLE]
    # cparams_blosc2['filters_meta'] = [1]
    # cparams_blosc2['splitmode'] = blosc2.SplitMode.NEVER_SPLIT

    # Open the dataset
    f = h5py.File('/Users/faltet/Downloads/lung_raw_2000-2100.h5', 'r')
    dset = f['/data']
    print(f"*** Compressing dataset of {dset.shape} images ...")

    for cratio in range(1, 11):
        print(f"*** Compressing with cratio={cratio}x ...")
        # Set the parameters that will be used by grok
        kwargs = {
            'cod_format': blosc2_grok.GrkFileFmt.GRK_FMT_JP2,
            'num_threads': 1,    # this does not have any effect (grok should work in multithreading mode)
            'quality_mode': "rates",
            'quality_layers': np.array([cratio], dtype=np.float64)
        }
        blosc2_grok.set_params_defaults(**kwargs)

        # for i in tqdm(range(dset.shape[0])):
        for i in range(1):  #dset.shape[0]):
            im = dset[i:i+1, ...]
            # Transform the numpy array to a blosc2 array. This is where compression happens.
            t0 = time()
            #blocks = (1, im.shape[1] // 4, im.shape[2] // 4)
            blocks = (1, im.shape[1], im.shape[2])
            b2im = blosc2.asarray(im, chunks=im.shape, blocks=blocks, cparams=cparams)
            t1 = time() - t0

            # Use a lossless codec for the original data
            t0 = time()
            b2im2 = blosc2.asarray(im, cparams=cparams_blosc2)
            t2 = time() - t0
            im2 = b2im[:]

            # Use a lossless codec for the compressed data with JPEG2000
            t0 = time()
            b2im3 = blosc2.asarray(im2, cparams=cparams_blosc2)
            t3 = time() - t0
            im3 = b2im3[:]

            if i == 0:
                # Compare with original
                ssim1 = ssim(im[0], im2[0], data_range=im.max() - im.min())
                cratio1 = b2im.schunk.cratio
                speed1 = (im.shape[1] * im.shape[2] * im.shape[0] * im.dtype.itemsize) / t1 / 1e6
                # Compare with lossless over original image
                ssim2 = ssim(im[0], im2[0], data_range=im.max() - im.min())
                cratio2 = b2im2.schunk.cratio
                speed2 = (im.shape[1] * im.shape[2] * im.shape[0] * im.dtype.itemsize) / t2 / 1e6
                # Compare with lossless over the compressed image with JPEG2000
                ssim3 = ssim(im[0], im3[0], data_range=im.max() - im.min())
                cratio3 = b2im3.schunk.cratio
                speed3 = (im.shape[1] * im.shape[2] * im.shape[0] * im.dtype.itemsize) / t3 / 1e6

        print(f"SSIM (grok): {ssim1:0.3f}")
        print(f"cratio (grok): {cratio1:.2f}")
        print(f"cspeed (grok): {speed1:.3f} MB/s")
        print(f"SSIM (zstd on orig image): {ssim2:0.3f}")
        print(f"cratio (zstd on orig image): {cratio2:.2f}")
        print(f"cspeed (zstd on orig image): {speed2:.3f} MB/s")
        print(f"SSIM (zstd on lossy image): {ssim3:0.3f}")
        print(f"cratio (zstd on lossy image): {cratio3:.2f}")
        print(f"cspeed (zstd on lossy image): {speed3:.3f} MB/s")

    f.close()
