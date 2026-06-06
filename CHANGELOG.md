##  (2026-06-06)


### Features

* Added AI generated workflow ([9d29f0a](https://github.com/Diamond42474/peregrine-constellation/commit/9d29f0aa2b12fc458709e75bc07a2ec3bbeb7349))
* Added auto-timing recovery ([92e4567](https://github.com/Diamond42474/peregrine-constellation/commit/92e4567c47341504ac8cbd07d8dd4f4b67bb62c3))
* Added bit unpacker is empty function ([d4e2475](https://github.com/Diamond42474/peregrine-constellation/commit/d4e24758a71e619487ed0fb913d761ffb883ef81))
* Added bit unpacker to the modem ([0daf52b](https://github.com/Diamond42474/peregrine-constellation/commit/0daf52b1dab9dda47bd4e3f62026ed3f96edb23f))
* Added BSP interfaces for audio in/out and PTT control ([f81b4f7](https://github.com/Diamond42474/peregrine-constellation/commit/f81b4f765dde4256a31dc57a4e8ef6fda0ba5c70))
* Added RX handling so now we process samples and send decoded packets to the orchestrator ([4abb44d](https://github.com/Diamond42474/peregrine-constellation/commit/4abb44dabd713f116371aa785dab999ed74ab218))
* Added set head function for circular buffer for use with DMA operations ([f52a219](https://github.com/Diamond42474/peregrine-constellation/commit/f52a2193ded662bc95c007145fd72e8dfd61be71))
* Added time abstration and utilities ([7e35cbf](https://github.com/Diamond42474/peregrine-constellation/commit/7e35cbffe0bc601a3ac97c427b20d4797e14491f))
* Added Unity testing framework ([fd0f4ef](https://github.com/Diamond42474/peregrine-constellation/commit/fd0f4ef00d3bedb649eea5b460e3694e8d9a5322))
* AFSK using bandpass filtering, envelope detection, and PLL for constant timing recovery ([730da32](https://github.com/Diamond42474/peregrine-constellation/commit/730da3233faf592f910f8ff07fb42f461abf8432))
* built out cobs decoder pipeline and integrated it into decoder pipeline manager ([595ef48](https://github.com/Diamond42474/peregrine-constellation/commit/595ef485ccc787c018f1ed9d273846b84c5a52fd))
* Encoder Pipeline ([cc19e52](https://github.com/Diamond42474/peregrine-constellation/commit/cc19e52ebcaed1f9cfe12796f1faf3b211602dd0))
* Implemented modem TX ([47d531e](https://github.com/Diamond42474/peregrine-constellation/commit/47d531e48d05aaa90e03c30142a57d068d41a36e))
* Made biquad filtering dynamic to sample rate ([a74681e](https://github.com/Diamond42474/peregrine-constellation/commit/a74681e928aa595086d39dfa848ef34f472c809e))
* Pipeline for managing frame encoding ([883af6d](https://github.com/Diamond42474/peregrine-constellation/commit/883af6df9f1de4dfc9251751c8d8056eb13c5a0e))
* PSUDO Send/Receive ([13dfb85](https://github.com/Diamond42474/peregrine-constellation/commit/13dfb85739e544767a7c5a0e3c2df03cf975f3ec))
* Started adding in a modem and orchestrator to handle packets and sending and receiving data ([7371ee9](https://github.com/Diamond42474/peregrine-constellation/commit/7371ee91560988d5b770a834e1f4b7da08ad0a0f))


### Bug Fixes

* Bad MSB/LSB handling when dealing with uint16_t preamble ([288d2c3](https://github.com/Diamond42474/peregrine-constellation/commit/288d2c3399dc398ecfaa2a31be648361b68da01a))
* byte assembler preamle takes into account bit order ([3dc15f7](https://github.com/Diamond42474/peregrine-constellation/commit/3dc15f754beff1f19e13b48fb39fdeb9ddfa7e91))
* byte assembler sends preamble so framer knows when new message starts ([24e343f](https://github.com/Diamond42474/peregrine-constellation/commit/24e343f17da9539ca787342d2de6ed1e9938f239))
* Fixed bit unpacker bug that never told us when we no longer have a byte ([a652bc4](https://github.com/Diamond42474/peregrine-constellation/commit/a652bc4e6fc0c84f86520c0555807d8544a55b51))
* Fixed bug that was caused by not setting the decoder input buffer size ([9e3948b](https://github.com/Diamond42474/peregrine-constellation/commit/9e3948b359734c12d685c0133d3579ad595356fa))
* now decoder doesn't consider available frame to be a pending transfer since the decoder doesn't handle the data after that. (it's grabbed from a high level) ([d3ffbdc](https://github.com/Diamond42474/peregrine-constellation/commit/d3ffbdcdb57fd23d570276a8f96d71deafa7bf68))


### Performance Improvements

* Added state-based flags for grabbing when the decoder is busy or not ([30c9bcc](https://github.com/Diamond42474/peregrine-constellation/commit/30c9bcc2da0f6f1f22c2384aa32c8321b47a0cb8))
* Decoder now uses single input and output buffer that's shared between sub-modules ([df63923](https://github.com/Diamond42474/peregrine-constellation/commit/df639237c8097ec9aceca616e93f625bf0fa6950))
* Faster FSK window offset calculation ([cc67878](https://github.com/Diamond42474/peregrine-constellation/commit/cc6787837b8de2af79ffbd67a8923a0438bd0316))
* simplified fsk timing calculator ([9759f0c](https://github.com/Diamond42474/peregrine-constellation/commit/9759f0ca30091dcae83014938cc93e22219ec267))

