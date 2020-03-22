# Another Finite State Machine

`afsm` is a finite state machine C++11 library designed for usage in multithreaded asynchronous environment.

## Inspiration and Motivation

The `afsm` library was inspired by [`::boost::msm`](http://www.boost.org/doc/libs/1_62_0/libs/msm/doc/HTML/index.html) library and implemented so that the migration from `::boost::msm` was a bunch of search and replace operations. The main motivation was to create a thread-safe FSM library and to achieve decent compile times for large and complex state machines not sacrificing performance. A state machine defined with `afms` library compiles several times faster than same library defined with `::boost::msm` and has similar (or better) performance. You can find some benchmark results [here](https://github.com/zmij/afsm/wiki/Performance-Benchmarks).

## Features

* Statechart features
  * Hierarchical states
  * [Entry and exit actions](https://github.com/zmij/afsm/wiki/Entry-and-Exit-Actions)
  * Internal transitions
  * [Transition actions](https://github.com/zmij/afsm/wiki/Transition-Actions)
  * [Transition guards (conditions)](https://github.com/zmij/afsm/wiki/Transition-Guards)
  * [State history](https://github.com/zmij/afsm/wiki/History)
  * [Event deferring](https://github.com/zmij/afsm/wiki/Event-Deferring)
  * [Orthogonal regions](https://github.com/zmij/afsm/wiki/Orthogonal-Regions)
* Statechart extensions
  * Optional [event priority](https://github.com/zmij/afsm/wiki/Event-Priority)
  * Optional [common base](https://github.com/zmij/afsm/wiki/Common-Base) for states and easy definition of dispatching common interface calls to current state
  * [Pushdown automaton](https://github.com/zmij/afsm/wiki/Pushdown-Automaton)
* Compile-time checks
* [Thread safety](https://github.com/zmij/afsm/wiki/Thread-Safety)
* Exception safety
* No vtables (unless common base feature is used)
* Header only
* Relatively fast compile time
* No external dependencies except STL

### Planned features

* State machine persistense

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
    minimal fsm;
    fsm.process_event(start{});
    fsm.process_event(stop{});
}
```

You can find a tutorial covering most of basic features [here](https://github.com/zmij/afsm/wiki/Tutorial:-Vending-machine-FSM).

## Documentation

Please see [project wiki](https://github.com/zmij/afsm/wiki) for documentation. *TODO* doxygen generated documentation.

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
