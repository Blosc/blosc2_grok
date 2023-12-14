##############################################################################
# blosc2_grok: Grok (JPEG2000 codec) plugin for Blosc2
#
# Copyright (c) 2023  The Blosc Development Team <blosc@blosc.org>
# https://blosc.org
# License: GNU Affero General Public License v3.0 (see LICENSE.txt)
##############################################################################

import numpy as np
import pytest
from PIL import Image
import time


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
        ({'enableTilePartGeneration': True}),
        ({'tile_size': (1009, 1000)}),
        ({'tile_size': (1009, 1000), 'enableTilePartGeneration': True}),
        ({'numgbits': 4}),
        ({'progression': 'RLCP'}),
        ({'progression': 'RPCL'}),
        ({'progression': 'PCRL'}),
        ({'progression': 'CPRL'}),
        ({'num_resolutions': 8}),
        ({'irreversible': True}),
        ({'codeblock_size': (4, 4)}),
        ({'codeblock_size': (8, 64)}),
        ({'codeblock_size': (256, 8)}),
        ({'roi_compno': 0}),
        ({'roi_compno': 1}),
        ({'roi_compno': 2}),
        ({'roi_compno': 3}),
        ({'roi_shift': 8}),
        ({'precinct_size': (32, 32)}),
        ({'precinct_size': (64, 64)}),
        ({'precinct_size': (128, 128)}),
        ({'offset': (33, 40)}),
        ({'mct': 1}),
        ({'mct': 1, 'irreversible': True}),
        ({'max_cs_size': 256}),
        ({'max_cs_size': 256, 'quality_mode': 'rates', 'quality_layers': np.array([5], dtype=np.float64)}),
        ({'max_comp_size': 10**9}),
        ({'rsiz': blosc2_grok.GrkProfile.GRK_PROFILE_0}),
        ({'rsiz': blosc2_grok.GrkProfile.GRK_PROFILE_1}),
        # ({'framerate': 8}), # Would make sense if we had more than one frame
        ({'apply_icc_': True}),
        # ({'numThreads': 4}),  # Deactivated until we can actually use it
        # ({'deviceId': 8}),  # Meant for multi-GPU systems
        ({'duration': 1}),
        ({'repeats': 2}),
        ({'repeats': 0}),
        ({'sharedMemoryInterface': True}),
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
    print(np_array.shape)

    if kwargs.get('mct', 0) == 1 and np_array.ndim != 3:
        pytest.skip("YCC conversion is only meant to be used for RGB")

    # Set the parameters that will be used by the codec
    blosc2_grok.set_params_defaults(**kwargs)

    cparams = {
        'codec': 160,
        'filters': [],
        'splitmode': blosc2.SplitMode.NEVER_SPLIT,
    }

    start = time.time()
    bl_array = blosc2.asarray(
        np_array,
        chunks=np_array.shape,
        blocks=np_array.shape,
        cparams=cparams,
    )
    stop = time.time()
    if kwargs.get('duration', 0) > 0:
        assert stop - start < kwargs['duration']

    print(bl_array.schunk.cratio)
    if kwargs.get('quality_mode', None) is None:
        if kwargs.get('max_cs_size', None) is None and not kwargs.get('irreversible', False):
            np.testing.assert_array_equal(bl_array[...], np_array)
    else:
        if kwargs['quality_mode'] == 'rates' and kwargs.get('max_cs_size', None) is None:
            assert bl_array.schunk.cratio >= kwargs['quality_layers'][0] - 0.1
        _ = bl_array[...]
