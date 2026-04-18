# shorkmatrix

My minimalist, blue-themed take on [Abishek V Ashok's CMatrix](https://github.com/abishekvashok/cmatrix). shorkmatrix is a "digital rain" vertical scrolling text screensaver, inspired by the 1999 film "The Matrix". The film and its subsequent franchise used similar visualisations of falling computer code to depict the activity of the titular simulated reality environment. The "droplets" of this digital rain are lines of blue characters that fall from the top of the terminal to the bottom, and are occasionally broken up. This implementation is designed to be more performant on SHORK Operating Systems like [SHORK 486](https://github.com/SharktasticA/SHORK-486). It also works on modern Linux distributions just fine.

**Disclaimer:** shorkmatrix is an unofficial, non-commercial and fan-made homage to The Matrix. It is not endorsed by, sponsored by, or affiliated with Warner Bros. or The Matrix franchise. All trademarks, logos, and copyrights are the property of their respective owners. No copyright infringement is intended.



## Building

### Requirements

You just need a C compiler (tested with GCC with either glibc or musl).

### Compilation

Simply run `make` to compile.

### Installation

Run `make install` to install to `/usr/bin` (you may need `sudo` if not installing as root). If you want to install it elsewhere, you can override the install location prefix like `make PREFIX=/usr/local install`.

**Note:** If you have a package maintainer's version of the original sl installed, or have compiled and installed the original sl yourself, this may conflict and/or overwrite it.



## Running

Simply run `shorkmatrix`.

### Arguments

* `-h`, `--help`: Shows help information and exits
* `-g`, `--green`: Changes the droplet colour to green
* `-ma`, `--magenta`: Changes the droplet colour to magenta
* `-mo`, `--mono`: Disables colour support and lighter heads
* `-nc`, `--no-clear`: Prevents clearing the terminal before starting
* `-r`, `--red`: Changes the droplet colour to red
* `-sh`, `--single-head`: Makes the lighter head of the droplets one character long instead of two
* `-u`, `--update`: Custom draw update control value (be must positive whole number)
* `-y`, `--yellow`: Changes the droplet colour to yellow
