# Release notes

## Changes from 0.3.2 to 0.3.3

* Change the Python extension from MODULE to SHARED on some
  platforms (Linux and MacOSX/arm64; the rest do not seem
  to support SHARED mode).  This allows for a C program to
  use the plugin as a shared library.

## Changes from 0.3.1 to 0.3.2

* Support for arbitrary numbers of leading 1 dimensions in the input data.
  This is common in image data where the leading dimensions are reserved for
  stacks of images.

## Changes from 0.3.0 to 0.3.1

* Build aarch64 wheels.


## Changes from 0.2.3 to 0.3.0

* Support specifying the `rates` value as the
 `codec_meta` from the blosc2 cparams.


## Changes from 0.2.2 to 0.2.3

* Fixed a memory leak in the decoder function.


## Changes from 0.2.1 to 0.2.2

* Changed initialization of the grok library
  to first time it is used. This evicts having to import
  the `blosc2-grok` package to use the plugin.


## Changes from 0.2.0 to 0.2.1

* Avoid calling `set_params_defaults` for setting own blosc2_grok defaults.


## Changes from 0.1.0 to 0.2.0

* Default `cod_format` changed to JP2.
* Added `mode` param to perform high throughput coding.
* Added some benchmarks.
* Added include header in `utils.h`.


## Changes from 0.0.1 to 0.1.0

* Added support for many parameters for the grok codec.
* Documentation for params added in the README.
* Fixed a bug when compressing several images in a row.
* Sporadic segfaults when compressing/decompressing fixed.
* First public release.
