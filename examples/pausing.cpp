/*
 * minimal.cpp
 *
 *  Created on: Nov 14, 2016
 *      Author: zmij
 */

#include <afsm/fsm.hpp>

#include <iostream>

namespace pausing {

// Events
struct start {};
struct stop {};
struct pause {};

// State machine definition
struct pausing_def : ::afsm::def::state_machine<pausing_def> {
    struct initial : state<initial> {};
    struct running : state<running> {};
    struct paused : state<paused> {};
    struct terminated : terminal_state<terminated> {};

    using initial_state = initial;
    // clang-format off
    using transitions   = transition_table<
        /*  State       Event       Next        */
        tr< initial,    start,      running     >,
        tr< running,    pause,      paused      >,
        tr< paused,     stop,       terminated  >,
        tr< running,    stop,       terminated  >
    >;
    // clang-format on
};

// State machine object
using pausing = ::afsm::state_machine<pausing_def>;

void
use()
{
    pausing fsm;
    fsm.process_event(start{});
    fsm.process_event(pause{});
    fsm.process_event(start{});
    fsm.process_event(stop{});
}

} /* namespace pausing */

int
main(int, char*[])
try {
    pausing::use();
    return 0;
} catch (::std::exception const& e) {
    ::std::cerr << "Exception: " << e.what() << "\n";
    return 1;
} catch (...) {
    ::std::cerr << "Unexpected exception\n";
    return 2;
}
