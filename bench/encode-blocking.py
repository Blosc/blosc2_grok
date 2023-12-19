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
    # Register grok codec locally
    blosc2.register_codec('grok', 160)

    # Define the compression and decompression parameters. Disable the filters and the
    # splitmode, because these don't work with the codec.
    cparams = {
        'codec': 160,
        'nthreads': 4,
        'filters': [],
        'splitmode': blosc2.SplitMode.NEVER_SPLIT,
    }

    # Open the dataset
    f = h5py.File('/Users/faltet/Downloads/lung_raw_2000-2100.h5', 'r')
    dset = f['/data']
    print(f"Compressing dataset of {dset.shape} images ...")

    for cratio in range(1, 11):
        print(f"Compressing with cratio={cratio}x ...")
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
            time_ = time() - t0
            if i == 0:
                # Compare with original
                im2 = b2im[:]
                ssim_ = ssim(im[0], im2[0], data_range=im.max() - im.min())
                cratio = b2im.schunk.cratio
        print(f"SSIM: {ssim_}")
        print(f"cratio: {cratio}")
        print(f"time: {time_}")

    f.close()
