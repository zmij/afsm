/*
 * vending_starter.cpp
 *
 *  Created on: Nov 15, 2016
 *      Author: zmij
 */

#include <iostream>
#include <afsm/fsm.hpp>

namespace vending {

namespace events {

struct power_on {};
struct power_off {};

}  /* namespace events */

struct vending_def : ::afsm::def::state_machine<vending_def> {
    //@{
    /** @name Substates definition */
    struct off : state<off> {};
    struct on  : state<on> {};
    //@}

    /** Initial state machine state */
    using initial_state = off;

    /** State transition table */
    using transitions = transition_table <
        /*  Start   Event               Next    */
        tr< off,    events::power_on,   on      >,
        tr< on,     events::power_off,  off     >
    >;
};

using vending_sm = ::afsm::state_machine<vending_def>;

::std::ostream&
operator << (::std::ostream& os, vending_sm const& val)
{
    ::std::ostream::sentry s(os);
    if (s) {
        os << (val.is_in_state< vending_sm::on >() ? "ON" : "OFF");
    }
    return os;
}


void
use()
{
    vending_sm vm;
    ::std::cout << "Machine is " << vm << "\n";
    vm.process_event(events::power_on{});
    ::std::cout << "Machine is " << vm << "\n";
    vm.process_event(events::power_off{});
    ::std::cout << "Machine is " << vm << "\n";
}

}  /* namespace vending */

int
main(int, char*[])
try {
    vending::use();
    return 0;
} catch (::std::exception const& e) {
    ::std::cerr << "Exception: " << e.what() << "\n";
    return 1;
} catch (...) {
    ::std::cerr << "Unexpected exception\n";
    return 2;
}
