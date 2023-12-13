# Blosc2 grok

A plugin of the excellent [grok library](https://github.com/GrokImageCompression/grok) for Blosc2.  grok is a JPEG2000 codec, and with this plugin, you can use it as yet another codec in applications using Blosc2.  See an example of use at: https://github.com/Blosc/blosc2_grok/blob/main/examples/params.py

## Installation

For using `blosc2_grok` you will first have to install its wheel:

```shell
pip install blosc2-grok -U
```

## Usage

```python
import blosc2
import numpy as np
import blosc2_grok
from PIL import Image

# Register the grok codec
blosc2.register_codec(blosc2_grok, 160)

# Set the params for the grok codec
kwargs = {}
kwargs['quality_mode'] = "dB"
kwargs['quality_layers'] = np.array([5], dtype=np.float64)
blosc2_grok.set_params_defaults(**kwargs)

# Define the compression and decompression parameters for Blosc2.
# Disable the filters and do not split blocks (these won't work with grok).
cparams = {
    'codec': 160,
    'filters': [],
    'splitmode': blosc2.SplitMode.NEVER_SPLIT,
}

# Read the image
im = Image.open("examples/kodim23.png")
# Convert the image to a numpy array
np_array = np.asarray(im)

# Transform the numpy array to a blosc2 array. This is where compression happens, and
# the HTJ2K codec is called.
bl_array = blosc2.asarray(
    np_array,
    chunks=np_array.shape,
    blocks=np_array.shape,
    cparams=cparams,
    urlpath="examples/kodim23.b2nd",
    mode="w",
)

# Print information about the array, see the compression ratio (cratio)
print(bl_array.info)
```

## Parameters for compression

The following parameters are available for compression for grok, with its defaults.  Most of them are named after the ones in the [Pillow library](https://pillow.readthedocs.io/en/stable/handbook/image-file-formats.html#jpeg-2000-saving) and have the same meaning.  The ones that are not in Pillow are marked with a `*` and you can get more information about them in the [grok documentation](https://github.com/GrokImageCompression/grok/wiki/3.-grk_compress).

    'tile_size': (0, 0),
    'tile_offset': (0, 0),
    # 'numlayers': 0, # blosc2_grok C func set_params will still receive this param
    'quality_mode': None,
    'quality_layers': np.zeros(0, dtype=np.float64),
    'csty': 0,
    'numgbits': 2,
    'progression': "LRCP",
    'num_resolutions': 6,
    'codeblock_size': (64, 64),
    'codeblock_style': 0,
    'irreversible': False, # blosc2_grok C func set_params will still receive this param
    'roi_compno': -1,
    'roi_shift': 0,
    'precinct_size': (0, 0),
    'offset': (0, 0),
    'subsampling_dx': 1,
    'subsampling_dy': 1,
    'decod_format': GrkFileFmt.GRK_FMT_UNK,
    'cod_format': GrkFileFmt.GRK_FMT_UNK,
    'enableTilePartGeneration': False,
    'newTilePartProgressionDivider': 0,
    'mct': 0,
    'max_cs_size': 0,
    'max_comp_size': 0,
    'rsiz': GrkProfile.GRK_PROFILE_NONE,
    'framerate': 0,
    'apply_icc_': False,
    'rateControlAlgorithm': GrkRateControl.BISECT,
    'numThreads': 0,
    'deviceId': 0,
    'duration': 0,
    'kernelBuildOptions': 0,
    'repeats': 1,
    'plt': False,
    'tlm': False,
    'verbose': False,
    'sharedMemoryInterface': False,

TODO: Complete this list, specially add the `*` marks.
TODO: Remove the ones that are not in Pillow nor in the grok page above. Or document them right here?

## More examples

See the [examples](examples/) directory for more examples.


That's all folks!

The Blosc Development Team
