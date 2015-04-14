#!/bin/bash

# Run this from the root directory of the "flipsta" repository.

# This script will generate a directory ../flipsta-build and not remove it!
# So watch out where you run it.

# Run the tests by using this repository as submodule of the "flipsta-build"
# repository.

# This is necessary because this repository cannot be tested by itself.


# Travis CI looks for this line.
set -ev

set -o nounset
set -o errexit

(
    # Check out the "master" branch of "flipsta-build" from GitHub, in the parent
    # directory.

    cd ../
    git clone git://github.com/rogiervd/flipsta-build.git
    cd flipsta-build
    git checkout master
    git submodule init
    git submodule update
    # Then replace the "flipsta" submodule with the one in ../flipsta.
    rm -r flipsta
    ln -s ../flipsta

    # Test it
    bjam test "$@"

)
