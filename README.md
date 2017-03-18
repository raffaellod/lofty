**Lofty** – coroutines, stack traces and smart I/O for C++11, inspired by Python and Golang. 

## 1. Introduction

Lofty is a C++11 framework featuring:

*  Multiple platform compatibility: Linux and Windows, partial with OS X and FreeBSD (see §
   _5. Compatibility_);

*  Built on top of the C++11 STL;

*  Coroutines and threads as easy as declaring a function, with a rich suite of I/O classes to enable truly
   asynchronous code;

*  Crash-free execution: every error condition (including dereferencing a null pointer) results in an
   exception, with support for integrated stack tracing (see `LOFTY_TRACE_FUNC()`);

*  An I/O class hierarchy that leaves the STL behind, making working with files and sockets a breeze – say
   goodbye to vulnerable `printf()` and clunky `cout`;

*  Testing framework integrated in the library, fully supported by the recommended build tool,
   [Complemake](https://github.com/raffaellod/complemake);

*  Full support for Unicode and the C++11 `char32_t` character type (see `lofty::text`).


## 2. Getting Lofty

Lofty is [available on GitHub](https://github.com/raffaellod/lofty); to download it, just clone the
repository:

```
git clone https://github.com/raffaellod/lofty.git
cd lofty
```

See § _4. Versioning and branching_ for more information on available branches.


### 2.1. Building

Building Lofty requires [Complemake](https://github.com/raffaellod/complemake), which works as an improved
make utility and dependency manager.

To build Lofty, run Complemake from Lofty’s repo clone:

```
complemake build
```

This will create outputs in the `bin` and `lib` folders (you can change that with Complemake’s flags).


### 2.2. Installing

At the moment, Lofty lacks any means for installation. If you’re using Complemake to build your project,
Complemake will take care of making Lofty available to projects that declare it as a dependency. You’ll still
need to manipulate the environment to run any binaries though (see § _3. Using Lofty_).

**TODO**: make Lofty installable.


## 3. Using Lofty

For usage examples, please see the source files in the `examples` folder. Examples are built as part of Lofty
(see § _2.1. Building Lofty_), and the generated executables can be found in the `bin` output folder.

If Lofty is not installed (see § _4. Installing Lofty_), programs will fail to run due to being unable to load
Lofty shared libraries; this can be worked around by adding the output `lib` folder to the shared library
search path:

*  On glibc/Linux and FreeBSD:
   ```
   env LD_LIBRARY_PATH=$PWD/lib bin/hello-world
   ```

*  On OS X:
   ```
   env DYLD_LIBRARY_PATH=$PWD/lib bin/hello-world
   ```

*  On Windows:
   ```
   set PATH=%PATH%;%CD%\lib
   bin\hello-world
   ```


## 4. Versioning and branching

Lofty uses semantic versioning: _MAJOR.MINOR.REVISION_ .

While the major version number is 0, changes to the minor indicate breaking changes, while the revision is
incremented for non-breaking changes such as bug fixes and minor improvements.

Version 1.0.0 will indicate the first production-grade release, and the meaning of the versioning schema will
shift accordingly: the major number will indicate breaking changes, the minor non-breaking changes (e.g. for
maintenance releases), and the revision will be incremented for bug fixes and other minor improvements.

Revisions are tags named `vX.Y.Z` where _X_ is the major version, _Y_ is the minor version, and _Z_ is the
revision.

Versions are branches named `vX.Y` where _X_ is the major version and _Y_ is the minor version. Version
branches start with a revision tag, and may have more revision tags for maintenance releases.

Currently there are no version branches, as long as the major version remains 0.

The default branch is `master`; this branch tracks the most common ancestor of all development branches, which
are one for each platform:

*  Linux:   `linux`
*  Windows: `win`
*  OS X:    `osx` (currently stale due to lack of active development)
*  FreeBSD: `freebsd`

When Lofty will have version branches, each version branch should proably get a development branch of its own
in the form of `platform_vX.Y`, with the version branch `vX.Y` being a tracker in the same way `master` tracks
the main development branches.


## 5. Compatibility

Lofty is in full development, so its compatibility can and will change over time (hopefully expanding).

Supported build systems:

*  GNU toolchain
   *  GCC 4.7 to 5.2
   *  binutils 2.20 or later

*  Microsoft Visual Studio 2010-2013 (Visual C++ 10-12 / MSC 16-18)

Clang/LLVM is not fully supported at this moment due to missing compiler features. If and when these will get
integrated in LLVM, Lofty will support these additional build systems:

*  Clang + GNU LD
   *  Clang 3.5
   *  binutils 2.20 or later

*  Apple SDK for OS X 10.10 Yosemite and 10.9 Mavericks (included in Xcode 6)

Supported operating systems:

*  GNU/Linux 2.6 or later, using glibc 2.17 or later;
*  Microsoft Windows – currently via Visual C++ 10 or later, which means Windows XP and later versions are
   supported.

These operating systems are supported at the source code level, but binaries built for them are fundmentally
flawed due to the above-mentioned LLVM limitations:

*  OS X 10.9 Mavericks or later;
*  FreeBSD – officially only the latest -RELEASE is supported.

Additionally, Lofty requires Python 2.7 or 3.2 or later to be installed on the build host system.


## 6. Past, present and future


### 6.1. Some history

Lofty was started in the mid-2000s with a dual scope: on one hand explore if it was possible, (ab)using
templates and other C++ features, to have automatic error reporting similar to what higher-level languages
such as Python and Java offer; on the other hand, a required target was to be able to write programs that
would be light, fast, easy to write, and easy to read years later.

At the time, the features of C++ were insufficient to accomplish all that without using some sort of
pre-preprocessor, so the project goals lead to several dead-ended projects.

When C++11 finally reached the final draft status, it turned out that it allowed for most of the features that
had originally been planned, so development of Lofty as it is now was finally started.


### 6.2. Current status of Lofty

Lofty can be considered of “alpha” grade, and tinkering with it is strongly encouraged.


### 6.3. Project goals

Lofty has very ambitious goals (hence its name):

1. Supersede other C++ libraries that fail to exploit the features of C++11 – especially libraries developed
   as C++ bindings for C libraries;

2. Demonstrate that C++ programs can be as sleek as Java or Python programs, while being considerably more
   efficient;

3. Encourage writing of software in C++ instead of other less efficient languages such as C (development
   efficiency) or Java (execution efficiency).

All future development will be geared towards getting closer to accomplishing these objectives.




--------------------------------------------------------------------------------------------------------------
Copyright 2010-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License along with Lofty. If not, see
http://www.gnu.org/licenses/ .
