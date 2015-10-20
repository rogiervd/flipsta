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

BRANCH=${1}
shift

# If the branch is "master", check out the master branch of range-test.
# Otherwise, check out the develop branch of range-test.
if [ x${BRANCH} != xmaster ]
then
    BRANCH=develop
fi

(
    # Check out the "master" branch of "flipsta-build" from GitHub, in the
    # parent directory.

    cd ../
    git clone git://github.com/rogiervd/flipsta-build.git
    cd flipsta-build
    echo Checking out branch ${BRANCH} of flipsta-build...
    git checkout ${BRANCH}
    git submodule init
    git submodule update
    # Then replace the "flipsta" submodule with the one in ../flipsta.
    rm -r flipsta
    ln -s ../flipsta

    # Run the tests.
    bjam "$@"

)
