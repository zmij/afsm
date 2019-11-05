/*
 * minimal.cpp
 *
 *  Created on: Nov 14, 2016
 *      Author: zmij
 */

#include <afsm/fsm.hpp>

#include <iostream>

namespace minimal {

// Events
struct start {};
struct stop {};

// State machine definition
struct minimal_def : ::afsm::def::state_machine<minimal_def> {
    struct initial : state<initial> {
        template <typename Event, typename FSM>
        void
        on_exit(Event const&, FSM&)
        {
            std::cout << "exit initial\n";
        }
    };
    struct running : state<running> {
        template <typename Event, typename FSM>
        void
        on_enter(Event const&, FSM&)
        {
            std::cout << "enter running\n";
        }
        template <typename Event, typename FSM>
        void
        on_exit(Event const&, FSM&)
        {
            std::cout << "exit running\n";
        }
    };
    struct terminated : terminal_state<terminated> {
        template <typename Event, typename FSM>
        void
        on_enter(Event const&, FSM&)
        {
            std::cout << "enter terminated\n";
        }
    };

    using initial_state = initial;
    // clang-format off
    using transitions   = transition_table<
        /*  State       Event       Next        */
        tr< initial,    start,      running     >,
        tr< running,    stop,       terminated  >
    >;
    // clang-format on
};

// State machine object
using minimal = ::afsm::state_machine<minimal_def>;

void
use()
{
    minimal fsm;
    fsm.process_event(start{});
    fsm.process_event(stop{});
}

} /* namespace minimal */

int
main(int, char*[])
try {
    minimal::use();
    return 0;
} catch (std::exception const& e) {
    std::cerr << "Exception: " << e.what() << "\n";
    return 1;
} catch (...) {
    std::cerr << "Unexpected exception\n";
    return 2;
}
