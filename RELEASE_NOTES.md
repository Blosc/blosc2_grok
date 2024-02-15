# Release notes

## Changes from 0.2.3 to 0.2.4

#XXX version-specific blurb XXX#


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
