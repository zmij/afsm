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
    operator()(event_a&& evt, FSM& fsm, SourceState& source, TargetState& target) const
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

struct inner_transitions_test : def::state< inner_transitions_test > {
    ::std::string value = "none";

    using internal_transitions = def::transition_table <
        in< event_a,    dummy_action,   a_guard >,
        in< event_a,    dummy_action_a, meta::not_< a_guard > >,
        in< event_b,    dummy_action,   none >,
        in< event_c,    dummy_action,   none >
    >;
};

using test_state = state<none, inner_transitions_test>;

TEST(FSM, InnerStateTransitions)
{
    none n;
    test_state ts{n};
    EXPECT_EQ("none", ts.value);
    EXPECT_EQ(detail::event_process_result::process, ts.process_event(event_a{}));
    EXPECT_EQ("a", ts.value);
    EXPECT_EQ(detail::event_process_result::process, ts.process_event(event_b{}));
    EXPECT_EQ("b", ts.value);
    EXPECT_EQ(detail::event_process_result::process, ts.process_event(event_c{}));
    EXPECT_EQ("c", ts.value);
}

}  /* namespace a */

}  /* namespace test */
}  /* namespace afsm */
