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
from pathlib import Path


import blosc2
import blosc2_grok

project_dir = Path(__file__).parent.parent
@pytest.mark.parametrize('image', [project_dir / 'examples/kodim23.png', project_dir / 'examples/MI04_020751.tif'])
def test_def_params(image):
    im = Image.open(image)
    # Convert the image to a numpy array
    np_array = np.asarray(im)
    print(np_array.shape)

    cparams = {
        'codec': blosc2.Codec.GROK,
        'filters': [],
        'splitmode': blosc2.SplitMode.NEVER_SPLIT,
    }

    bl_array = blosc2.asarray(
        np_array,
        chunks=np_array.shape,
        blocks=np_array.shape,
        cparams=cparams,
    )
    
    np.testing.assert_array_equal(bl_array[...], np_array)
