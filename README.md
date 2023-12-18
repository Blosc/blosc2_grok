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
blosc2.register_codec("grok", 160)

# Set the params for the grok codec
kwargs = {}
kwargs['cod_format'] = blosc2_grok.GrkFileFmt.GRK_FMT_JP2
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

The following parameters are available for compression for grok, with their defaults.  Most of them are named after the ones in the [Pillow library](https://pillow.readthedocs.io/en/stable/handbook/image-file-formats.html#jpeg-2000-saving) and have the same meaning.  The ones that are not in Pillow are marked with a `*` and you can get more information about them in the [grok documentation](https://github.com/GrokImageCompression/grok/wiki/3.-grk_compress), or by following the provided links.

    'tile_size': (0, 0),
    'tile_offset': (0, 0),
    'quality_mode': None,
    'quality_layers': np.zeros(0, dtype=np.float64),
    'progression': "LRCP",
    'num_resolutions': 6,
    'codeblock_size': (64, 64),
    'irreversible': False,
    'precinct_size': (0, 0),
    'offset': (0, 0),
    'mct': 0,
    * 'numgbits': 2,  # Equivalent to -N, -guard_bits
    * 'roi_compno': -1,  # Together with 'roi_shift' it is equivalent to -R, -ROI
    * 'roi_shift': 0,
    * 'decod_format': GrkFileFmt.GRK_FMT_UNK,
    * 'cod_format': GrkFileFmt.GRK_FMT_UNK,
    * 'rsiz': GrkProfile.GRK_PROFILE_NONE,  # Equivalent to -Z, -rsiz
    * 'framerate': 0,
    * 'apply_icc_': False,  # Equivalent to -f, -apply_icc
    * 'rateControlAlgorithm': GrkRateControl.BISECT,
    * 'num_threads': 0,
    * 'deviceId': 0,  # Equivalent to -G, -device_id
    * 'duration': 0,  # Equivalent to -J, -duration
    * 'repeats': 1,  # Equivalent to -e, -repetitions
    * 'verbose': False,
    * 'enableTilePartGeneration': False,  # See https://github.com/GrokImageCompression/grok/blob/a84ac2592e581405a976a00cf9e6f03cab7e2481/src/lib/core/grok.h#L975
    * 'max_cs_size': 0,  # See https://github.com/GrokImageCompression/grok/blob/a84ac2592e581405a976a00cf9e6f03cab7e2481/src/lib/core/grok.h#L975
    * 'max_comp_size': 0,  # See https://github.com/GrokImageCompression/grok/blob/a84ac2592e581405a976a00cf9e6f03cab7e2481/src/lib/core/grok.h#L975

## More examples

See the [examples](examples/) directory for more examples.

## Thanks

Thanks to Aaron Boxer, the original author of the [grok library](https://github.com/GrokImageCompression/grok), for his help in making this plugin possible.  Also, thanks to Marta Iborra, from the Blosc Development Team, for doing most of the job in making this plugin possible.

That's all folks!

The Blosc Development Team
