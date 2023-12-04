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

That's all folks!
