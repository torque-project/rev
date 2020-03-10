# rev
A Clojure interpreter. This is useful as a bootstrap stage for other Clojure VMs and compilers

The interpreter is currently able to run all of torque.core, which is a full adaption of clojure.core (state 2016).

## Getting Started

To build simply clone the repository recursively to fetch all sub repositories.

    git clone --recursive -b develop git@github.com:torque-project/rev.git
    
Switch into the *lib/libffi* directory. build, and install the library to a suitable location, where the compiler can see it, like */usr/local*. After that you can just run make in the project root.

    make -j4
    
After this step the interpreter resides in the target directory, in an system/architecture dependent sub folder. To start up a repl with clojure.core support, run 

    export REV_SOURCE_PATHS=lib/core/src 
    target/<arch>/bin/booti
    
This should give you a full clojure repl. You can load your own libraries by setting REV_SOURCE_PATH to a colon seprated list of paths. See any of the torque libraries for an overview of how to structure a library. If you want to play with the torque compiler, or I/O libraries, check out the other repoistories in this project.
