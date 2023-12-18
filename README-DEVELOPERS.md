# blosc2_grok

For using blosc2_grok you will first have to create and install its wheel.

## Create the wheel

For Linux:

```shell
python -m cibuildwheel --only 'cp311-manylinux_x86_64'
```

For Mac x86_64:

```shell
CMAKE_OSX_ARCHITECTURES=x86_64 python -m cibuildwheel --only 'cp311-macosx_x86_64'
```

For Mac arm64:

```shell
CMAKE_OSX_ARCHITECTURES=arm64 python -m cibuildwheel --only 'cp311-macosx_arm64'
```

## Install the wheel

```shell
pip install wheelhouse/blosc2_grok-*.whl --force-reinstall
```

## Debugging

If you would like to debug and run an example from C getting to track the problem through the C functions, you can use
the codec as a local registered codec. For that you will have to do the following:

```
// In blosc2_grok_public.h
// Comment out the info
//BLOSC2_GROK_EXPORT codec_info info = {
//    .encoder=(char *)"blosc2_grok_encoder",
//    .decoder=(char *)"blosc2_grok_decoder"
//};

// In your example, include the blosc2_grok_public.h header and add the function pointers
// to the codec struct before registering it.
#include "blosc2_grok_public.h"
// Some code in between
blosc2_codec grok_codec = {0};
grok_codec.compname = (char *)"grok";
grok_codec.compcode = 160;
grok_codec.complib = 1;
grok_codec.version = 0;
grok_codec.encoder = &blosc2_grok_encoder;
grok_codec.decoder = &blosc2_grok_decoder;
int rc = blosc2_register_codec(&grok_codec);
```

That's all folks!
