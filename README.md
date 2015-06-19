# CppTemplate

CppTemplate is a C++ project template for use in creating new command-line utility projects using PuppetLabs' shared libraries.

## Required packages

You will need to install [Boost](http://boost.org) for program_options and to use many of the libraries in [Leatherman](https://github.com/puppetlabs/leatherman).

## Creating a new project

This is most simply accomplished with [Hub](https://hub.github.com/). The following commands will create a new GitHub project based on this template named `new-project` under the `puppetlabs` organization. Use Hub's `-p` option to create a private repo.

```
hub clone --depth 1 --origin source puppetlabs/cpp-project-template new-project
cd new-project
hub create puppetlabs/new-project [-p]
```

## Build the library

This template is a fully functional example, and can be built with

```
git submodule update --init
cmake .
make
```

Tests can be run with `make test`, and the example tool can be run with `bin/example`.
