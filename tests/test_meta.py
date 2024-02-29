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
from pathlib import Path


import blosc2

project_dir = Path(__file__).parent.parent
@pytest.mark.parametrize('image', [project_dir / 'examples/kodim23.png', project_dir / 'examples/MI04_020751.tif'])
@pytest.mark.parametrize('meta', [1 * 10, 2 * 10, 5 * 10, 8 * 10, 10 * 10, 20 * 10])
def test_meta(image, meta):
    im = Image.open(image)
    # Convert the image to a numpy array
    np_array = np.asarray(im)

    # Set the parameters that will be used by the codec
    cparams = {
        'codec': blosc2.Codec.GROK,
        'codec_meta': meta,
        'filters': [],
        'splitmode': blosc2.SplitMode.NEVER_SPLIT,
    }

    bl_array = blosc2.asarray(
        np_array,
        chunks=np_array.shape,
        blocks=np_array.shape,
        cparams=cparams,
    )
    print(bl_array.schunk.cratio)

    assert bl_array.schunk.cratio >= meta / 10 - 0.1
