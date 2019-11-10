/**
 * Copyright 2019 Sergei A. Fedorov
 * machine_gun.cpp
 *
 *  Created on: Nov 4, 2019
 *      Author: ser-fedorov
 */

#include "machine_gun.hpp"

#include "../test/test_observer.hpp"

#include <iostream>

using machine_gun
    = afsm::state_machine<guns::machine_gun_def, afsm::none, afsm::test::test_fsm_observer>;

int
main(int, char*[])
{
    machine_gun mg;

    mg.set_on_emit_bullet([]() { std::clog << "pew\n"; });
    mg.set_on_reload([]() { std::clog << "reload\n"; });
    mg.set_on_empty([]() { std::clog << "ammo depleted\n"; });
    mg.set_on_safety_change([](guns::safety_lever v) { std::clog << to_chars(v) << "\n"; });

    mg.make_observer("AK47");
    mg.process_event(afsm::none{});
    mg.process_event(guns::events::safety_lever_down{});
    mg.process_event(guns::events::reload{});
    mg.process_event(guns::events::trigger_pull{});
    while (!mg.empty()) {
        mg.process_event(guns::events::tick{});
    }
    mg.process_event(guns::events::trigger_release{});

    return 0;
}
