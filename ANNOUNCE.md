Announcing blosc2-grok 0.2.0
============================

This is a minor release, were the `cod_format` default value was change to JP2 
and the `mode` param was added to activate high throughput.
Furthermore, with the new Python-Blosc2 release, there is no need to register the
codec. Instead, just use its id `blosc2.Codec.GROK`.
For more info, you can have a look at the release notes in:

https://github.com/Blosc/blosc2_grok/releases

More info and examples are available in the README:

https://github.com/Blosc/blosc2_grok#readme

## What is it?

Blosc2-grok is a dynamic codec plugin for Blosc2 that allows to compress
and decompress images using the JPEG 2000 standard.  For details, check the
[grok wiki](https://github.com/GrokImageCompression/grok/wiki).

## Sources repository

The sources and documentation are managed through github services at:

https://github.com/Blosc/blosc2_grok

blosc2_grok is distributed using the AGPL license, see
https://github.com/Blosc/blosc2_grok/blob/main/LICENSE.txt
for details.


## Mastodon feed

Please follow https://fosstodon.org/@Blosc2 to get informed about the latest
developments.

- The Blosc Development Team
  We make compression better
