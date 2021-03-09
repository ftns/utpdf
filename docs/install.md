# Installation note
<div style="text-align: right;">
3/9/2021<br>
Akihiro SHIMIZU
</div>

## Required module and it's package name.

- Cairo
~~~
    Debian:   libcairo2
    FreeBSD:  cairo
    Homebrew: cairo
~~~

- Pango
~~~
    Debian:   libpango-1.0, libpango1.0-dev
    FreeBSD:  pango
    Homebrew: pango
~~~
- pkg-config
~~~
    Debian:   pkg-config
    FreeBSD:  pkgconf
    Homebrew: pkg-config
~~~
- GNU make
~~~
    FreeBSD:  gmake
~~~

## Installation method

1. Install above modules (by `apt/pkg/brew` ...etc.).
2. Check and edit Makefile as needed.<br>
   For example on FreeBSD, `MANDIR` must be changed like:<br>
   `MANDIR = /usr/local/man`
3. With GNU make, do:<br>
   `$ make && sudo make install`


## International fonts

Due to pango limitaion, available fonts seems to be their family name 
contain only alphabet, number, minus, and space -- `[A-Za-z0-9\ \-]+` 
in regular expression.

With a shell script `font_candidate.sh` included in this package, dump
fonts name matched above condition like this:<br>
`$ font_candidate.sh <lang>`<br>
	`<lang>`     : language 2-letter code, such as `ja, th, vi, tr, zh...`
