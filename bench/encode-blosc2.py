##############################################################################
# blosc2_grok: Grok (JPEG2000 codec) plugin for Blosc2
#
# Copyright (c) 2023  The Blosc Development Team <blosc@blosc.org>
# https://blosc.org
# License: GNU Affero General Public License v3.0 (see LICENSE.txt)
##############################################################################

"""
Benchmark for compressing a dataset of images with the grok codec and store it into Blosc2.

Data can be downloaded from: http://www.silx.org/pub/nabu/data/compression/lung_raw_2000-2100.h5
"""

import h5py
import blosc2
import blosc2_grok
import numpy as np
from tqdm import tqdm


IMAGES_PER_CHUNK = 16

if __name__ == '__main__':
    # Define the compression and decompression parameters. Disable the filters and the
    # splitmode, because these don't work with the codec.
    cparams = {
        'codec': blosc2.Codec.GROK,
        'nthreads': 8,
        'filters': [],
        'splitmode': blosc2.SplitMode.NEVER_SPLIT,
    }

    # Open the dataset
    f = h5py.File('/Users/faltet/Downloads/lung_raw_2000-2100.h5', 'r')
    dset = f['/data']
    print(f"Compressing dataset of {dset.shape} images ...")

    # for cratio in range(1, 11):
    #for cratio in [4, 6, 8, 10, 20]:
    for cratio in [1]:
        print(f"Compressing with cratio={cratio}x ...")
        # Set the parameters that will be used by grok
        kwargs = {
            'cod_format': blosc2_grok.GrkFileFmt.GRK_FMT_JP2,
            'num_threads': 1,    # this does not have any effect (grok should work in multithreading mode)
            'quality_mode': "rates",
            'quality_layers': np.array([cratio], dtype=np.float64)
        }
        blosc2_grok.set_params_defaults(**kwargs)

        # Open the output file
        chunks = (IMAGES_PER_CHUNK,) + dset.shape[1:]   # IMAGES_PER_CHUNK images per chunk
        blocks = (1,) + dset.shape[1:]   # 1 image per block
        fout = blosc2.uninit(dset.shape, dset.dtype, chunks=chunks, blocks=blocks, # cparams=cparams,
                             urlpath=f'/Users/faltet/Downloads/lung_jpeg2k_2000-2100-{cratio}x.b2nd',
                             mode='w')

        # Do the actual transcoding
        for i in tqdm(range(0, dset.shape[0], IMAGES_PER_CHUNK)):
            im = dset[i:i+IMAGES_PER_CHUNK, ...]
            # Transform the numpy array to a blosc2 array. This is where compression happens.
            fout[i:i+IMAGES_PER_CHUNK] = im

    f.close()
