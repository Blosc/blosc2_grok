##############################################################################
# blosc2_grok: Grok (JPEG2000 codec) plugin for Blosc2
#
# Copyright (c) 2023  The Blosc Development Team <blosc@blosc.org>
# https://blosc.org
# License: GNU Affero General Public License v3.0 (see LICENSE.txt)
##############################################################################

"""
Benchmark for compressing a dataset of images with the grok codec and store it into HDF5.

Data can be downloaded from: http://www.silx.org/pub/nabu/data/compression/lung_raw_2000-2100.h5
"""

import h5py
import blosc2
import blosc2_grok
import numpy as np
import hdf5plugin
from skimage.metrics import structural_similarity as ssim
from tqdm import tqdm


if __name__ == '__main__':
    # Register grok codec locally
    blosc2.register_codec('grok', 160)

    # Define the compression and decompression parameters. Disable the filters and the
    # splitmode, because these don't work with the codec.
    cparams = {
        'codec': 160,
        'nthreads': 1,
        'filters': [],
        'splitmode': blosc2.SplitMode.NEVER_SPLIT,
    }

    # Open the dataset
    f = h5py.File('/Users/faltet/Downloads/lung_raw_2000-2100.h5', 'r')
    dset = f['/data']
    print(f"Compressing dataset of {dset.shape} images ...")

    # Compression params for identifying the codec. In the future, one should be able to
    # specify the rok plugin (and its parameters) here.
    b2params = hdf5plugin.Blosc2()

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

        # Open the output file
        fout = h5py.File(f'/Users/faltet/Downloads/lung_jpeg2k_2000-2100-{cratio}x.h5', 'w')
        chunks = (1,) + dset.shape[1:]
        dset_out = fout.create_dataset('/data', shape=dset.shape, dtype=dset.dtype, chunks=chunks, **b2params)

        for i in tqdm(range(dset.shape[0])):
            im = dset[i:i+1, ...]
            # Transform the numpy array to a blosc2 array. This is where compression happens.
            b2im = blosc2.asarray(im, chunks=im.shape, blocks=im.shape, cparams=cparams)

            # Write to disk
            dset_out.id.write_direct_chunk((i, 0, 0), b2im.schunk.to_cframe())
            if i == 0:
                # Compare with lossless
                # Decompress from memory
                #im2 = b2im[:]
                # Read and decompress
                #im2 = dset_out[i]  # does not work yet (needs to be registered in blosc2/hdf5plugin)
                # Workaround with read_direct_chunk
                _, im2 = dset_out.id.read_direct_chunk((i, 0, 0))
                im2 = blosc2.ndarray_from_cframe(im2)
                im2 = im2[:]
                ssim_lossy = ssim(im[0], im2[0], data_range=im.max() - im.min())
        print(f"SSIM lossy: {ssim_lossy}")
        fout.close()

    f.close()
