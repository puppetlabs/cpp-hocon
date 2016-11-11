# C++ HOCON Parser

![Travis CI](https://travis-ci.org/puppetlabs/cpp-hocon.svg)
![Appveyor](https://ci.appveyor.com/api/projects/status/github/puppetlabs/cpp-hocon?svg=true)



This is a port of the TypesafeConfig library to C++.

The library provides C++ support for the [HOCON configuration file format](https://github.com/typesafehub/config/blob/master/HOCON.md).

```
          MMMMMMMMMMMMMMMMMMMM
         .====================.
         MMMMMMMMMMMMMMMMMMMMMM
        .MMMMMMMMMMMMMMMMMMMMMM.
    ===.7MMMIN7NMMMMMMMMM7M=MMMM,===
    MMM.7MM:     DMMMMM7    :MMM=MMM
    MMM.7MM,     DMMMMM?    ~MMM=MMM
    MMM.7MMM~++~?MMMMMMM~++~MMMM=MMM
        .MMMMMMMMMMMMMMMMMMMMMM.
         MMMMMMMMMMMMMMMMMMMMMM
         MMMMMMMMMMMMMMMMMMMMMM
         MMM      HOCON     MMM
         MMMMMMMMMMMMMMMMMMMMMM
          .MMMMMMMMMMMMMMMMMM.
          .MMMMMMMMMMMMMMMMMM.
       .MMMMMMMMMMMMMMMMMMMMMMMM
 .    MMMMMMMMMMMMM88MMMMMMMMMM8MM    .
7=MMMMMMMM++ A CONFIG FILE  ++M8MMMMMM7=
M=MMMMMMMM+ FORMAT DESIGNED  +M8MMMMMM7M
M=MMMMMMMM++  FOR HUMANS   ++M8MMMMMM7M
 =MMMMMMMMMMMMMMMMM88MMMMMMMMMM8MMMMMM7
  7MM.88MMMMMMMMMMM88MMMMMMMMMMO88 MM8
  7MM   MMMMMMMMMMM88MMMMMMMMMM8   MM8
  7MM   MMMMMMMMMMM88MMMMMMMMMM8   MM8
  7MM   MMMDMMMM?MM88MM?MMMMOMM8   MM8
```


## Build Requirements

* OSX or Linux
* GCC >= 4.8 or Clang >= 3.4 (with libc++)
* CMake >= 3.2.2
* Boost Libraries >= 1.54
* [Leatherman](https://github.com/puppetlabs/leatherman)


## Pre-Build

Prepare the cmake release environment:

    $ mkdir release
    $ cd release
    $ cmake ..


Optionally, also prepare the debug environment:

    $ mkdir debug
    $ cd debug
    $ cmake -DCMAKE_BUILD_TYPE=Debug ..


## Building

1. Enter your build environment of choice, i.e. `cd release` or `cd debug`
2. `make`
3. (optional) install with `make install`

## Testing

Run tests with `make test`.
