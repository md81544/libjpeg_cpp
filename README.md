# libjpeg_cpp
C++ wrapper around (parts of) libjpeg.
See main.cpp for example usage.

See also https://www.martyndavis.com/?p=684

## Dependencies

On Linux, or WSL, install libjpeg with

```
sudo apt install libjpeg-dev
```

On Mac, use

```
brew install jpeg
```

Note for Mac: You will need to ensure that the include/library paths used by homebrew
are included, I put the following in my `~/.zshrc` file:

```
export CPATH=$(brew --prefix)/include
export LIBRARY_PATH=$(brew --prefix)/lib
```