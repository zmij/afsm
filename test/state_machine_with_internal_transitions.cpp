/*
 * state_machine_with_internal_transitions.cpp
 *
 *  Created on: Nov 14, 2016
 *      Author: zmij
 */

#include "test_observer.hpp"
#include <afsm/fsm.hpp>

#include <gtest/gtest.h>

namespace afsm {
namespace test {

namespace events {

struct a_to_b {};
struct b_to_a {};
struct in_state {};

} /* namespace events */

struct outer_machine : def::state_machine<outer_machine> {
    struct inner_action {
        template <typename FSM>
        void
        operator()(events::in_state const&, FSM&) const
        {}
    };
    struct transit_action {
        template <typename Event, typename FSM>
        void
        operator()(Event const&, FSM&) const
        {}
    };
    struct state_a : def::state<state_a> {};
    struct state_b : def::state<state_b> {
        using deferred_events = type_tuple<events::a_to_b>;
    };

    using initial_state        = state_a;
    using internal_transitions = def::transition_table<in<events::in_state, inner_action, none>>;
    // clang-format off
    using transitions           = def::transition_table<
        //in< events::in_state,   inner_action,   none    >,
        tr< state_a,    events::a_to_b,     state_b,    transit_action,     none    >,
        tr< state_b,    events::b_to_a,     state_a,    transit_action,     none    >
    >;
    // clang-format on
};

} /* namespace test */
// Instantiate it
template class state_machine<test::outer_machine, none, test::test_fsm_observer>;

namespace test {

using test_sm = state_machine<outer_machine, none, test_fsm_observer>;

TEST(FSM, MachineWithInstate)
{
    test_sm tsm;
    tsm.make_observer("test");
    // *** INITIAL STATE ***
    // Check current state
    EXPECT_TRUE(tsm.is_in_state<outer_machine::state_a>());
    EXPECT_FALSE(tsm.is_in_state<outer_machine::state_b>());
    // Process in-state
    EXPECT_EQ(actions::event_process_result::process_in_state,
              tsm.process_event(events::in_state{}));
    // Check current state
    EXPECT_TRUE(tsm.is_in_state<outer_machine::state_a>());
    EXPECT_FALSE(tsm.is_in_state<outer_machine::state_b>());
    // *** A -> B ***
    // Process with transition
    EXPECT_EQ(actions::event_process_result::process, tsm.process_event(events::a_to_b{}));
    // Check current state
    EXPECT_TRUE(tsm.is_in_state<outer_machine::state_b>());
    EXPECT_FALSE(tsm.is_in_state<outer_machine::state_a>());
    // Process in-state
    EXPECT_EQ(actions::event_process_result::process_in_state,
              tsm.process_event(events::in_state{}));
    // Check current state
    EXPECT_TRUE(tsm.is_in_state<outer_machine::state_b>());
    EXPECT_FALSE(tsm.is_in_state<outer_machine::state_a>());
    // *** B -> A ***
    // Process with transition
    EXPECT_EQ(actions::event_process_result::process, tsm.process_event(events::b_to_a{}));
    // Check current state
    EXPECT_TRUE(tsm.is_in_state<outer_machine::state_a>());
    EXPECT_FALSE(tsm.is_in_state<outer_machine::state_b>());
    // Process in-state
    EXPECT_EQ(actions::event_process_result::process_in_state,
              tsm.process_event(events::in_state{}));
}

TEST(FSM, MachineWithInstateDefers)
{
    test_sm tsm;
    tsm.make_observer("test");
    // *** INITIAL STATE ***
    // Check current state
    EXPECT_TRUE(tsm.is_in_state<outer_machine::state_a>());
    EXPECT_FALSE(tsm.is_in_state<outer_machine::state_b>());
    // *** A -> B ***
    // Process with transition
    EXPECT_EQ(actions::event_process_result::process, tsm.process_event(events::a_to_b{}));
    // Check current state
    EXPECT_TRUE(tsm.is_in_state<outer_machine::state_b>());
    EXPECT_FALSE(tsm.is_in_state<outer_machine::state_a>());
    // Process in-state
    EXPECT_EQ(actions::event_process_result::process_in_state,
              tsm.process_event(events::in_state{}));
    // Check current state
    EXPECT_TRUE(tsm.is_in_state<outer_machine::state_b>());
    EXPECT_FALSE(tsm.is_in_state<outer_machine::state_a>());
    // Defer events
    EXPECT_EQ(actions::event_process_result::defer, tsm.process_event(events::a_to_b{}));
    EXPECT_EQ(actions::event_process_result::defer, tsm.process_event(events::a_to_b{}));
    // Check current state
    EXPECT_TRUE(tsm.is_in_state<outer_machine::state_b>());
    EXPECT_FALSE(tsm.is_in_state<outer_machine::state_a>());
    // Process in-state
    EXPECT_EQ(actions::event_process_result::process_in_state,
              tsm.process_event(events::in_state{}));
    // Check current state
    EXPECT_TRUE(tsm.is_in_state<outer_machine::state_b>());
    EXPECT_FALSE(tsm.is_in_state<outer_machine::state_a>());
    // *** B -> A -> B ***
    // Process with transition
    EXPECT_EQ(actions::event_process_result::process, tsm.process_event(events::b_to_a{}));
    // Check current state
    EXPECT_TRUE(tsm.is_in_state<outer_machine::state_b>());
    EXPECT_FALSE(tsm.is_in_state<outer_machine::state_a>());
    // Process in-state
    EXPECT_EQ(actions::event_process_result::process_in_state,
              tsm.process_event(events::in_state{}));
    // Check current state
    EXPECT_TRUE(tsm.is_in_state<outer_machine::state_b>());
    EXPECT_FALSE(tsm.is_in_state<outer_machine::state_a>());
    // *** B -> A -> B ***
    // Process with transition
    EXPECT_EQ(actions::event_process_result::process, tsm.process_event(events::b_to_a{}));
    // Check current state
    EXPECT_TRUE(tsm.is_in_state<outer_machine::state_b>());
    EXPECT_FALSE(tsm.is_in_state<outer_machine::state_a>());
}

} /* namespace test */
} /* namespace afsm */
