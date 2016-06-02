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
struct stateB : state< stateB > {};
struct stateC : state< stateC > {};

static_assert( traits::is_state<stateA>::value, "" );
static_assert( traits::is_state<stateB>::value, "" );
static_assert( traits::is_state<stateC>::value, "" );
static_assert( !traits::has_history<stateA>::value, "" );

struct stateD : state< stateD, void, tags::has_history > {};
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

}  /* namespace test */
}  /* namespace def */
}  /* namespace afsm */

