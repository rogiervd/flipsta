.. _building:

*********************
Building the library
*********************

The first question to ask before trying to build the library is: do you really need to?
You need to if

*   You would like to use the :ref:`Python interface <python>` and it is not possible to get a precompiled version.
*   You would like to use C++ to read or write files in AT&T (OpenFst) format or HTK lattice format.

Otherwise, to use the library in C++, adding the include paths ``<Flipsta root>/*/include`` on the C++ command line will do.
Note that if you use BJam, then this should be almost automatic.

.. rubric:: Actually building the library

Building the Flipsta library uses Boost.Build, which comes with the Boost libraries.
Boost.Build is a joy to use if it works, and, regretfully, a pain if it does not.
The main command this uses is ``bjam``.
Should you have this all set up, then go to the root directory of Flipsta distribution and say on the command line

::

    bjam

This should build the library, and

::

    bjam test

should run the unit tests.

Assuming this does not succeed, a number of dependencies need to be installed.
First, make sure you've got a C++ compiler installed.
Then, install the Boost libraries, and Boost.Build.
These are a set of well-known C++ libraries.
On Ubuntu, this will install them::

    sudo apt-get install libboost-all-dev bjam

If you are lucky, saying ``bjam`` should now work.

Quite likely, however, you need to edit ``~/user-config.jam`` (create it if it does not exist) to say::

    import boost ;
    boost.use-project ;

However, this requires the file ``boost.jam`` somewhere where ``bjam`` can find it.
Ubuntu does not seem to ship the automatic configuration files that would make this happen.
There are three ways to solve this:

1.  If you have administrator rights on your machine, this could be fixed as follows::

        wget https://github.com/boostorg/build/raw/develop/src/contrib/boost.jam
        sudo mkdir /usr/share/boost-build/contrib
        sudo cp ./boost.jam /usr/share/boost-build/contrib/boost.jam

    This allows Boost.Build to automatically find ``boost.jam`` when building the Flipsta library.

2.  Alternatively, download ``boost.jam`` to your home directory::

        wget https://github.com/boostorg/build/raw/develop/src/contrib/boost.jam -O ~/boost.jam

Now, the library should be built and tested when you say in the root directory of Flipsta::

    bjam
    bjam test
