##  (2025-11-04)


### Features

* Added AI generated workflow ([9d29f0a](https://github.com/Diamond42474/peregrine-constellation/commit/9d29f0aa2b12fc458709e75bc07a2ec3bbeb7349))
* built out cobs decoder pipeline and integrated it into decoder pipeline manager ([595ef48](https://github.com/Diamond42474/peregrine-constellation/commit/595ef485ccc787c018f1ed9d273846b84c5a52fd))


### Bug Fixes

* byte assembler preamle takes into account bit order ([3dc15f7](https://github.com/Diamond42474/peregrine-constellation/commit/3dc15f754beff1f19e13b48fb39fdeb9ddfa7e91))
* byte assembler sends preamble so framer knows when new message starts ([24e343f](https://github.com/Diamond42474/peregrine-constellation/commit/24e343f17da9539ca787342d2de6ed1e9938f239))
* now decoder doesn't consider available frame to be a pending transfer since the decoder doesn't handle the data after that. (it's grabbed from a high level) ([d3ffbdc](https://github.com/Diamond42474/peregrine-constellation/commit/d3ffbdcdb57fd23d570276a8f96d71deafa7bf68))


### Performance Improvements

* Added state-based flags for grabbing when the decoder is busy or not ([30c9bcc](https://github.com/Diamond42474/peregrine-constellation/commit/30c9bcc2da0f6f1f22c2384aa32c8321b47a0cb8))
* simplified fsk timing calculator ([9759f0c](https://github.com/Diamond42474/peregrine-constellation/commit/9759f0ca30091dcae83014938cc93e22219ec267))

