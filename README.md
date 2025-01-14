# C++ HOCON Parser

This repository is archived and Perforce will no longer be updating this repository. For more information, see [this Puppet blog post](https://www.puppet.com/blog/open-source-puppet-updates-2025).

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

To get started, [install it](#install), then to parse a file:
```
#include <hocon/parser/config_document_factory.hpp>
#include <fstream>

using hocon::config_document_factory::parse_file;

int main(int argc, char** argv) {
    auto doc = parse_file("file.conf");
    doc = doc->with_value_text("a", "42");

    std::ofstream out("file.conf");
    out << doc->render();
    return 0;
}
```

If you build cpp-hocon with `-DBUILD_SHARED_LIBS=ON`, then the example can be built with
```
c++ example.cc -o example -std=c++11 -lcpp-hocon
```

You can use `hocon::config_document_factory::parse_string` to parse a string. [config_document](lib/inc/hocon/parser/config_document.hpp) is used to modify a file while preserving all formatting. Use [config](lib/inc/hocon/config.hpp) to read from the config or if you don't care about preserving formatting.

Note that file extensions matter. A `.conf` file will be parsed as HOCON, a `.json` file will be parsed as JSON, and other extensions will be ignored.

See the [docs](https://puppetlabs.github.io/cpp-hocon) for more.

## Caveats

This is a mostly complete implementation of the HOCON format. It currently has some known limitations

* Include requires the location specifier, i.e. `include "foo"` won't work but `include file("foo")` will. URL is not yet implemented, and classpath won't be supported as it makes less sense outside of the JVM.
* Unicode testing is absent so support is unknown. There are likely things that won't work.

## Install

### Build Requirements

* OSX or Linux
* GCC >= 4.8 or Clang >= 3.4 (with libc++)
* CMake >= 3.2.2
* Boost Libraries >= 1.54
* [Leatherman](https://github.com/puppetlabs/leatherman)

### Pre-Build

Prepare the cmake release environment:

    $ mkdir release
    $ cd release
    $ cmake ..

Optionally, also prepare the debug environment:

    $ mkdir debug
    $ cd debug
    $ cmake -DCMAKE_BUILD_TYPE=Debug ..

### Building

1. Enter your build environment of choice, i.e. `cd release` or `cd debug`
2. `make`
3. (optional) install with `make install`

### Testing

Run tests with `make test`.
