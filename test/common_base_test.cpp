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
struct sleep {};

struct human_interface {
    virtual ~human_interface() {}

    virtual void
    work() = 0;

    virtual void
    sleep() = 0;
};

struct dummy_action {
    template< typename FSM, typename SourceState, typename TargetState >
    void
    operator()(wash const&, FSM&, SourceState&, TargetState&)
    {
        ::std::cerr << "Brrr!\n";
    }
    template< typename FSM, typename SourceState, typename TargetState >
    void
    operator()(do_work const&, FSM&, SourceState&, TargetState&)
    {
        ::std::cerr << "Enough work!\n";
    }
};

struct sleep_action {
    template < typename FSM, typename SourceState, typename TargetState >
    void
    operator()(sleep const&, FSM& fsm, SourceState&, TargetState&) const
    {
        if (fsm.fatigue > 0)
            --fsm.fatigue;
    }
};

struct work_action {
    template < typename FSM, typename SourceState, typename TargetState >
    void
    operator()(do_work const&, FSM& fsm, SourceState&, TargetState&) const
    {
        ::std::cerr << "Getting tired! " << ++fsm.fatigue << "\n";
    }
};

struct human_def : ::afsm::def::state_machine< human_def, false, human_interface > {
    struct sleeping : state<sleeping> {
        void
        work() override
        { ::std::cerr << "Zzzzzz!\n"; }
        void
        sleep() override
        { ::std::cerr << "ZzzZ.Zzzz!\n"; }

        using internal_transitions = transition_table <
            in< test::sleep,      sleep_action,     none >
        >;
    };

    struct awake : state_machine<awake, false, human_interface> {
        struct woken_up : state<woken_up> {
            using deferred_events = type_tuple< do_work >;
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

            using internal_transitions = transition_table<
                in< do_work, work_action, none >
            >;
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

        struct is_tired {
            template < typename FSM, typename State >
            bool
            operator()(FSM const& fsm, State const&) const
            {
                ::std::cerr << "Check tired " << fsm.fatigue << "\n";
                return fsm.fatigue >= 5;
            }
        };

        using initial_state = woken_up;
        using transitions = transition_table<
            tr< woken_up,   wash,       fresh,      dummy_action                >,
            tr< fresh,      do_work,    tired,      dummy_action,   is_tired    >,
            tr< tired,      food,       sleepy,     none                        >
        >;

        awake()
            : fatigue{0}
        {
            ::std::cerr << "Construct awake\n";
        }
        void
        work() override
        { ::std::cerr << "TODO Access common base of current state\n"; }
        void
        sleep() override
        { ::std::cerr << "TODO Access common base of current state\n"; }

        int fatigue = 0;
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

    int fatigue = 0;
};

using human_fsm = ::afsm::state_machine<human_def>;

TEST(FSM, CommonBase)
{
    using afsm::actions::event_process_result;
    human_fsm hfsm;
    EXPECT_EQ(event_process_result::process, hfsm.process_event(alarm{}));
    EXPECT_EQ(event_process_result::defer, hfsm.process_event(do_work{}));
    EXPECT_EQ(event_process_result::defer, hfsm.process_event(do_work{}));
    EXPECT_EQ(event_process_result::defer, hfsm.process_event(do_work{}));
    EXPECT_EQ(event_process_result::process, hfsm.process_event(wash{}));
    EXPECT_EQ(event_process_result::process_in_state, hfsm.process_event(do_work{}));
    EXPECT_EQ(event_process_result::process_in_state, hfsm.process_event(do_work{}));
    EXPECT_EQ(event_process_result::process, hfsm.process_event(do_work{}));
}

} // namespace test
