#######################################################################
# Copyright (c) 2019-present, Blosc Development Team <blosc@blosc.org>
# All rights reserved.
#
# This source code is licensed under a BSD-style license (found in the
# LICENSE file in the root directory of this source tree)
#######################################################################

import numpy as np
import pytest
from PIL import Image

import blosc2
import blosc2_grok


@pytest.mark.parametrize('image', ['examples/kodim23.png', 'examples/MI04_020751.tif'])
@pytest.mark.parametrize(
    'args',
    [
        ({}),
        ({'quality_mode': 'rates', 'quality_layers': np.array([5], dtype=np.float64)}),
        ({'quality_mode': 'rates',
          'quality_layers': np.array([5], dtype=np.float64),
          'rateControlAlgorithm': blosc2_grok.GrkRateControl.PCRD_OPT}
        ),
        ({'quality_mode': 'dB', 'quality_layers': np.array([5], dtype=np.float64)}),
        # ({'enableTilePartGeneration': True}),
        ({'tile_size': (100, 100)}),
        # ({'tile_size': (100, 100), 'enableTilePartGeneration': True}),
        # ({'csty': 1}),
        ({'progression': 'RLCP'}),
        ({'progression': 'RPCL'}),
        ({'progression': 'PCRL'}),
        ({'progression': 'CPRL'}),
    ],
)
def test_jp2(image, args):
    kwargs = {'cod_format': blosc2_grok.GrkFileFmt.GRK_FMT_JP2,
              'verbose': True,
              }
    kwargs.update(args)

    # Register codec locally for now
    blosc2.register_codec('grok', 160)

    im = Image.open(image)
    # Convert the image to a numpy array
    np_array = np.asarray(im)
    # Set the parameters that will be used by the codec
    blosc2_grok.set_params_defaults(**kwargs)

    cparams = {
        'codec': 160,
        # 'nthreads': nthreads,
        'filters': [],
        'splitmode': blosc2.SplitMode.NEVER_SPLIT,
    }

    bl_array = blosc2.asarray(
        np_array,
        chunks=np_array.shape,
        blocks=np_array.shape,
        cparams=cparams,
    )

    tile_size = kwargs.get('tile_size', None)
    if tile_size is not None:
        im.save(image + '.jp2', tile_size=tile_size)
        im2 = Image.open(image + '.jp2')
        np_array2 = np.asarray(im2)
        np.testing.assert_array_equal(bl_array[...], np_array2)
        return

    progression = kwargs.get('progression', None)
    if progression is not None:
        im.save(image + '.jp2', progression=progression)
        im2 = Image.open(image + '.jp2')
        np_array2 = np.asarray(im2)
        np.testing.assert_array_equal(bl_array[...], np_array2)
        return

    if kwargs.get('quality_mode', None) is None:
        _ = bl_array[...]
        np.testing.assert_array_equal(bl_array[...], np_array)
    else:
        if kwargs['quality_mode'] == 'rates':
            assert bl_array.schunk.cratio >= kwargs['quality_layers'][0] - 0.1
        _ = bl_array[...]
