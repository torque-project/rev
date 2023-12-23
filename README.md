# rev

A Clojure interpreter. This is useful as a bootstrap stage for other Clojure VMs and compilers

# Introduction

This repository is part of the torque project, which is a from scratch Clojure VM. 
It contains a full Clojure interpreter, that is used during the boot strapping stage
of the full compiler written in Clojure itself.

The interpreter is currently able to run all of torque.core, which is a full adaption
of clojure.core (Clojure 1.8).

## Supported Platforms

Currently tested on *Linux*, and *Darwin*.

# Getting Started

To build simply clone the repository recursively to fetch all sub repositories.

    git clone --recursive -b develop git@github.com:torque-project/rev.git
    
Switch into the *lib/libffi* directory. build, and install the library to a suitable location, 
where the compiler can see it, like */usr/local*. After that you can just run make in 
the project root.

    make -j12
    
After this step the interpreter resides in the target directory, in an system/architecture 
dependent sub folder. To start up a repl with clojure.core support, run 

    export REV_SOURCE_PATHS=lib/core/src 
    target/<arch>/bin/booti
    
This should give you a full clojure repl. You can load your own libraries by setting
REV_SOURCE_PATH to a colon seprated list of paths.
See any of the torque libraries for an overview of how to structure a library.
If you want to play with the torque compiler, or I/O libraries, check out the
other repoistories in this project.

# Roadmap

The interpreter functions pretty well at the moment, but could be extended of 
course. The big open issues are:

* Mainly because it's used as a bootstrap tool, for the actual compiler,
output formatting, and exceptions are bare bones, and could use some 
improvement. 
* The interpreter has no memory management/garbage collection, so will overflow
with time. This is again due to it mainly being used as a bootstrapping tool. I
would ultimately like to have some sort of garbage colleciton in the interpreter though.

