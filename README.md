# Video Indexer
C++/Qt project to find spoken speech in videos using libav/FFmpeg and CMUSphinx

# Environment Setup
To setup your environment to build the project, follow these instructions:  
* Install the required libraries:
```
sudo apt-get install mesa-common-dev freeglut3 freeglut3-dev libpulse-dev
```
* Download the CMUSphinx source files:
 * [PocketSphinx](https://sourceforge.net/projects/cmusphinx/files/pocketsphinx/5prealpha/) and
 * [SphinxBase](https://sourceforge.net/projects/cmusphinx/files/sphinxbase/5prealpha/)

* Extract the `.tar.gz` for `sphinxbase`, and run:
```
sudo apt-get install automake autoconf libtool bison swig
```

* Go to the `sphinxbase` folder, and build and install using:
```
./configure
make
sudo make install
```

* Export the following paths:
```
export LD_LIBRARY_PATH=/usr/local/lib
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
```

* Build and install `pocketsphinx` in the same way as `sphinxbase`.

* Download and install Qt5, and run the installer.

* Install `libav`, and copy `avconv` to the current directory.

You should have an environment ready to build the project
