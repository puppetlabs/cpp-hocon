# Change Log
All notable changes to this project will be documented in this file.
This project adheres to [Semantic Versioning](http://semver.org/).

## [0.2.0] - TBD

### Summary

Fixed minor bugs and irritations.

### Features
- Bumped Leatherman submodule from bc900e3d494a3932f97e90b8c6d5916690295a24 to a1627940f269b65537ee1b5f87c0624866218fd3.
- Added specific git commit to @PROJECT_NAME_UPPER@_VERSION_WITH_COMMIT
- Split `cpplint` and `cppcheck` into their own Jenkins CI jobs.

### Fixes
- Fixed no-op acceptance Rake task for use in Jenkins CI.

## [0.1.0] - 2015-07-08

### Summary

Initial release of cpp-project-template, an example C++11 project.

### Features
- Basic command-line skeleton with dynamic library in C++.
- Travis and AppVeyor CI jobs, coveralls.io triggered from Travis, and no-op acceptance Rake tasks.
- Relies on the Leatherman C++ utility library.
