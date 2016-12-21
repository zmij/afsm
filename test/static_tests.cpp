/*
 * static_tests.cpp
 *
 *  Created on: May 26, 2016
 *      Author: zmij
 */

#include <gtest/gtest.h>
#include <afsm/fsm.hpp>

namespace afsm {
namespace def {
namespace test {

struct state_interface {
};

struct eventAB{};
struct eventBC{};
struct eventAC{};

struct stateA : state< stateA > {};
struct stateB : state< stateB, tags::strong_exception_safety > {};
struct stateC : state< stateC, tags::nothrow_guarantee > {};

static_assert( traits::is_state<stateA>::value, "" );
static_assert( traits::is_state<stateB>::value, "" );
static_assert( traits::is_state<stateC>::value, "" );
static_assert( !traits::has_history<stateA>::value, "" );

struct stateD : state< stateD, tags::has_history > {};
static_assert( traits::is_state<stateD>::value, "" );
static_assert( traits::has_history<stateD>::value, "" );

struct stateE : state< stateE, state_interface > {};
static_assert( traits::is_state<stateE>::value, "" );
static_assert( !traits::has_history<stateE>::value, "" );

struct stateF : state< stateF, state_interface, tags::has_history > {};
static_assert( traits::is_state<stateF>::value, "" );
static_assert( traits::has_history<stateF>::value, "" );

static_assert(::std::is_same<
         detail::source_state< transition<stateA, eventAB, stateB> >::type,
         stateA>::value, "");
static_assert(::std::is_same<
         detail::target_state< transition<stateA, eventAB, stateB> >::type,
         stateB>::value, "");
using state_set_1 = ::psst::meta::unique<stateA, stateA, stateB, stateB, stateC, stateC>::type;
static_assert(state_set_1::size == 3, "");
static_assert(::psst::meta::contains< stateA, state_set_1 >::value, "");
static_assert(::psst::meta::contains< stateB, state_set_1 >::value, "");
static_assert(::psst::meta::all_match< traits::is_state, state_set_1 >::value, "");

using transition_table_1 = transition_table<
            transition<stateA, eventAB, stateB>,
            transition<stateA, eventAC, stateC>,
            transition<stateB, eventBC, stateC>
        >;
using empty_transition_table = transition_table<>;

static_assert(transition_table_1::inner_states::size == 3, "");
static_assert(::psst::meta::contains<stateA, transition_table_1::inner_states>::value, "");
static_assert(::psst::meta::contains<stateB, transition_table_1::inner_states>::value, "");
static_assert(::psst::meta::contains<stateC, transition_table_1::inner_states>::value, "");
static_assert(::psst::meta::all_match< traits::is_state, transition_table_1::inner_states >::value, "");
static_assert(transition_table_1::handled_events::size == 3, "");

static_assert(detail::has_transitions<transition_table_1>::value, "");
static_assert(!detail::has_transitions<empty_transition_table>::value, "");
static_assert(!detail::has_transitions<void>::value, "");
static_assert(detail::has_inner_states<transition_table_1>::value, "");
static_assert(!detail::has_inner_states<empty_transition_table>::value, "");

using events_in_state_a = ::psst::meta::find_if<
        def::originates_from<stateA>::template type,
        transition_table_1::transitions
    >;
using events_in_state_b = ::psst::meta::find_if<
        def::originates_from<stateB>::template type,
        transition_table_1::transitions
    >;
using events_in_state_c = ::psst::meta::find_if<
        def::originates_from<stateC>::template type,
        transition_table_1::transitions
    >;
static_assert(events_in_state_a::type::size == 2, "");
static_assert(events_in_state_b::type::size == 1, "");
static_assert(events_in_state_c::type::size == 0, "");

struct my_state : state <my_state> {
    using internal_transitions = transition_table <
        internal_transition< eventAB >,
        internal_transition< eventBC >
    >;
};
struct my_stable_state : state< my_stable_state > {
};

static_assert( !::std::is_same<my_state::internal_transitions, void>::value, "" );
static_assert(my_state::internal_transitions::transition_map::size == 2, "");
static_assert(my_state::internal_transitions::inner_states::size == 0, "");
static_assert(my_state::internal_transitions::handled_events::size == 2, "");
static_assert( ::std::is_same<my_stable_state::internal_transitions, void>::value, "" );
static_assert(!detail::has_inner_states<my_state::internal_transitions>::value, "");
static_assert(!detail::has_inner_states<my_stable_state::internal_transitions>::value, "");

struct my_fsm : state_machine<my_fsm> {
    using transitions = transition_table<>;
};

static_assert(traits::is_state<my_fsm>::value, "");
static_assert(traits::is_state_machine<my_fsm>::value, "");
static_assert(!traits::has_orthogonal_regions<my_fsm>::value, "");

struct ortho_fsm : state_machine<ortho_fsm> {
    struct region_a : state<region_a> {};
    struct region_b : state<region_b> {};
    using orthogonal_regions = type_tuple<region_a, region_b>;
};

static_assert(traits::is_state<ortho_fsm>::value, "");
static_assert(traits::is_state_machine<ortho_fsm>::value, "");
static_assert(traits::has_orthogonal_regions<ortho_fsm>::value, "");

//----------------------------------------------------------------------------
//  Actions
//----------------------------------------------------------------------------

struct action_long {
    template < typename Event, typename FSM, typename SourceState, typename TargetState >
    void
    operator()(Event&&, FSM&, SourceState&, TargetState&) {}
};

struct action_short {
    template < typename Event, typename FSM >
    void
    operator()(Event&&, FSM&) {}
};

static_assert(actions::detail::action_long_signature<action_long, eventAB, none, stateA, stateB>::value, "");
static_assert(!actions::detail::action_long_signature<action_short, eventAB, none, stateA, stateB>::value, "");

static_assert(!actions::detail::action_short_signature<action_long, eventAB, none>::value, "");
static_assert(actions::detail::action_short_signature<action_short, eventAB, none>::value, "");

//----------------------------------------------------------------------------
//  Exception safety
//----------------------------------------------------------------------------
static_assert(
    ::std::is_same<
        traits::exception_safety<stateA>::type,
        tags::basic_exception_safety
     >::value, "Default exception safety");
static_assert(
    ::std::is_same<
        traits::exception_safety<stateB>::type,
        tags::strong_exception_safety
     >::value, "Strong exception safety");
static_assert(
    ::std::is_same<
        traits::exception_safety<stateC>::type,
        tags::nothrow_guarantee
     >::value, "No-throw exception guarantee");

}  /* namespace test */
}  /* namespace def */
}  /* namespace afsm */

