#!/bin/bash
set -ev

# Set compiler to GCC 4.8 here, as Travis overrides the global variables.
export CC=gcc-4.8 CXX=g++-4.8

if [ ${TRAVIS_TARGET} == RELEASE ]; then
  cmake -Wno-dev .
  make cpplint
  make cppcheck
else
  cmake -Wno-dev -DCMAKE_BUILD_TYPE=Debug -DCOVERALLS=ON .
fi

make -j2
make test ARGS=-V

# Enable coveralls for public repos
#if [ ${TRAVIS_TARGET} == DEBUG ]; then coveralls --gcov gcov-4.8 --gcov-options '\-lp' -r .. >/dev/null; fi

