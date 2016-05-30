/*
 * common_base_test.cpp
 *
 *  Created on: 31 мая 2016 г.
 *      Author: sergey.fedorov
 */

#include <gtest/gtest.h>
#include <afsm/fsm.hpp>

#include <iostream>

namespace test {

struct wake {};
struct wash {};
struct food {};
struct do_work {};

struct alarm {};
struct pillow {};

struct human_interface {
    virtual ~human_interface() {}

    virtual void
    work() = 0;

    virtual void
    sleep() = 0;
};

struct human_def : ::afsm::def::state_machine< human_def, false, human_interface > {
    struct sleeping : state<sleeping> {
        void
        work() override
        { ::std::cerr << "Zzzzzz!\n"; }
        void
        sleep() override
        { ::std::cerr << "ZzzZ.Zzzz!\n"; }
    };

    struct awake : state_machine<awake, false, human_interface> {
        struct woken_up : state<woken_up> {
            void
            work() override
            { ::std::cerr << "Nay!\n"; }
            void
            sleep() override
            { ::std::cerr << "Nay!\n"; }

            using internal_transitions = transition_table<
                in< food, none, none >
            >;
        };
        struct fresh : state<fresh> {
            void
            work() override
            { ::std::cerr << "OK!\n"; }
            void
            sleep() override
            { ::std::cerr << "Nay!\n"; }
        };
        struct tired : state <tired> {
            void
            work() override
            { ::std::cerr << "Noooo!\n"; }
            void
            sleep() override
            { ::std::cerr << "Sooner the better!\n"; }
        };
        struct sleepy : state <sleepy> {
            void
            work() override
            { ::std::cerr << "Noooo...\n"; }
            void
            sleep() override
            { ::std::cerr << "Yaaawn!\n"; }
        };
        using initial_state = woken_up;
        using transitions = transition_table<
            tr< woken_up,   wash,       fresh    >,
            tr< fresh,      do_work,    tired    >,
            tr< tired,      food,       sleepy    >
        >;
        void
        work() override
        { ::std::cerr << "TODO Access common base of current state\n"; }
        void
        sleep() override
        { ::std::cerr << "TODO Access common base of current state\n"; }
    };

    using initial_state = sleeping;
    using transitions = transition_table <
        tr< sleeping,   alarm,      awake    >,
        tr< awake,      pillow,     sleeping >
    >;
    void
    work() override
    { ::std::cerr << "TODO Access common base of current state\n"; }
    void
    sleep() override
    { ::std::cerr << "TODO Access common base of current state\n"; }
};

using human_fsm = ::afsm::state_machine<human_def>;

TEST(FSM, CommonBase)
{
    human_fsm hfsm;
    hfsm.process_event(alarm{});

}

} // namespace test
