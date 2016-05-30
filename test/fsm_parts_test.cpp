/*
 * fsm_parts_test.cpp
 *
 *  Created on: 29 мая 2016 г.
 *      Author: sergey.fedorov
 */

#include <gtest/gtest.h>
#include <afsm/fsm.hpp>

namespace afsm {
namespace test {

namespace a {

struct event_a {};
struct event_b {};
struct event_c {};

struct dummy_action {
    template < typename FSM, typename SourceState, typename TargetState >
    void
    operator()(event_a&& evt, FSM& fsm, SourceState& source, TargetState& target) const
    {
        ::std::cerr << "Dummy action triggered (a)\n";
        source.value = "a";
    }
    template < typename FSM, typename SourceState, typename TargetState >
    void
    operator()(event_b&& evt, FSM& fsm, SourceState& source, TargetState& target) const
    {
        ::std::cerr << "Dummy action triggered (b)\n";
        source.value = "b";
    }
    template < typename FSM, typename SourceState, typename TargetState >
    void
    operator()(event_c&& evt, FSM& fsm, SourceState& source, TargetState& target) const
    {
        ::std::cerr << "Dummy action triggered (c)\n";
        source.value = "c";
    }
};

struct dummy_action_a {
    template < typename FSM, typename SourceState, typename TargetState >
    void
    operator()(event_a const& evt, FSM& fsm, SourceState& source, TargetState& target) const
    {
        ::std::cerr << "Dummy action 2 triggered (a)\n";
        source.value = "dummy";
    }
};

struct a_guard {
    template <typename FSM, typename State>
    bool
    operator()(FSM const&, State const&) const
    { return true; }
};

struct internal_transitions_test : def::state< internal_transitions_test > {
    ::std::string value = "none";

    using internal_transitions = transition_table <
        in< event_a,    dummy_action,   a_guard >,
        in< event_a,    dummy_action_a, meta::not_< a_guard > >,
        in< event_b,    dummy_action,   none >,
        in< event_c,    dummy_action,   none >
    >;
};

using test_state = state<none, internal_transitions_test>;

TEST(FSM, InnerStateTransitions)
{
    none n;
    test_state ts{n};
    EXPECT_EQ("none", ts.value);
    EXPECT_EQ(actions::event_process_result::process, ts.process_event(event_a{}));
    EXPECT_EQ("a", ts.value);
    EXPECT_EQ(actions::event_process_result::process, ts.process_event(event_b{}));
    EXPECT_EQ("b", ts.value);
    EXPECT_EQ(actions::event_process_result::process, ts.process_event(event_c{}));
    EXPECT_EQ("c", ts.value);
}

}  /* namespace a */

namespace b {

struct event_ab {};
struct inner_event {};

struct inner_dispatch_test : def::state_machine< inner_dispatch_test > {
    struct state_a;
    struct state_b;
    struct inner_action {
        template < typename FSM >
        void
        operator()(inner_event const& evt, FSM& fsm, state_a& source, state_a& target) const
        {
            ::std::cerr << "Dummy action triggered (inner_event - a)\n";
            fsm.value = "in_a";
        }
        template < typename FSM >
        void
        operator()(inner_event const& evt, FSM& fsm, state_b& source, state_b& target) const
        {
            ::std::cerr << "Dummy action triggered (inner_event - b)\n";
            fsm.value = "in_b";
        }
    };

    struct state_a : state< state_a > {
        using internal_transitions = def::transition_table <
            in< inner_event, inner_action, none >
        >;
    };

    struct state_b : state< state_b > {
        using internal_transitions = def::transition_table <
            in< inner_event, inner_action, none >
        >;
    };

    using transitions = transition_table <
        tr< state_a, event_ab, state_b >
    >;
    using initial_state = state_a;

    ::std::string value = "none";
};

using test_sm = inner_state_machine< none, inner_dispatch_test >;

TEST(FSM, InnerEventDispatch)
{
    none n;
    test_sm tsm{n};
    EXPECT_EQ("none", tsm.value);
    EXPECT_EQ(test_sm::initial_state_index, tsm.current_state());
    EXPECT_EQ(actions::event_process_result::process, tsm.process_event(inner_event{}));
    EXPECT_EQ("in_a", tsm.value);
    EXPECT_EQ(actions::event_process_result::refuse, tsm.process_event(event_ab{}));
}

}  /* namespace b */

}  /* namespace test */
}  /* namespace afsm */
