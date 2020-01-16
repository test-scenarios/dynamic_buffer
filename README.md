# Asio Dynamic Buffer

## Intention

A small project to test approaches to unifying the handling of
the asio concepts of DynamicBuffer_v1 and DynamicBuffer_v2


## Required toolchain

This program requires a c++11 or better toolchain.

Some very good toolchains files available here:

https://github.com/ruslo/polly

in which case you can invoke cmake with:

`cmake -DCMAKE_TOOLCHAIN_FILE=<POLLY_DIR>/cxx11.cmake -H<SRC_DIR> -B<BUILD_DIR>`

Where:

* `POLLY_DIR` is the cloned polly repo
* `SRC_DIR` is the directory containing this file
* `BUILD_DIR` is the intended build directory (in-source builds are evil)
