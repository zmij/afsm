# Another Finite State Machine

`afsm` is a finite state machine C++11 library designed for usage in multithreaded asynchronous environment.

## Inspiration and Motivation

The `afsm` library was inspired by [`::boost::msm`](http://www.boost.org/doc/libs/1_62_0/libs/msm/doc/HTML/index.html) library and implemented so that the migration from `::boost::msm` was a bunch of search and replace operations. The main motivation was to create a thread-safe FSM library and to achieve decent compile times for large and complex state machines.

## Features

* Statechart features
  * Hierarchical states
  * Entry and exit actions
  * Internal transitions
  * Transition actions
  * Transition guards (conditions)
  * State history
  * Event deferral
* Compile-time checks
* Thread safety
* Exception safety
* No vtables (unless common base feature is used)
* Optional common base for states and easy definition of dispatching calls to current state
* Header only
* Relatively fast compile time
* No external dependencies except STL

### Planned features

* Install targets for library headers #3
* Event priority #7
* Orthogonal regions #6
* Recursive state machine #8


## Synopsis

Here is a UML diagram of a trivial state machine and source code that it is mapped to.
![minimal](https://cloud.githubusercontent.com/assets/2694027/20274791/f352998c-aaa6-11e6-99ec-fc63300766d7.png)
```c++
#include <afsm/fsm.hpp>
// Events
struct start {};
struct stop {};

// State machine definition
struct minimal_def : ::afsm::def::state_machine<minimal_def> {
    //@{
    /** @name States */
    struct initial      : state<initial> {};
    struct running      : state<running> {};
    struct terminated   : terminal_state<terminated> {};
    //@}

    using initial_state = initial;
    using transitions   = transition_table<
        /*  State       Event       Next        */
        tr< initial,    start,      running     >,
        tr< running,    stop,       terminated  >
    >;
};

// State machine object
using minimal = ::afsm::state_machine<minimal_def>;

void use()
{
    mimimal fsm;
    fsm.process_event(start{});
    fsm.process_event(stop{});
}
```

You can find a tutorial covering most of basic features [here](https://github.com/zmij/afsm/wiki/Tutorial:-Vending-machine-FSM).

## Installation

The library is header only and doesn't requre build or installation. Just add the `afsm/include` and `lib/meta/include` directories under the root of this repository to your include paths.

### CMake subproject

You can add the library to your project as a subtree, e.g. `lib/afsm`, and in your root `CMakeLists.txt` file just do the following:

```cmake
add_subdirectory(lib/afsm)
include_directories(${AFSM_INCLUDE_DIRS})
```

TODO write docs on gitrc subtree commands and link to the repository

### Installation to System Directories

```bash
git clone git@github.com:zmij/afsm.git
mkdir afsm/build
cd afsm/build
cmake ..
sudo make install
```

### Finding the AFSM Package

```cmake
find_package(AFSM REQUIRED) # Will set AFSM_INCLUDE_DIRS variable
```

## License

[The Artistic License 2.0](https://github.com/zmij/afsm/blob/develop/LICENSE)
