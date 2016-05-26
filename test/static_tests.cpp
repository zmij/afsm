/*
 * static_tests.cpp
 *
 *  Created on: May 26, 2016
 *      Author: zmij
 */

#include <gtest/gtest.h>
#include <afsm/fsm.hpp>

namespace afsm {
namespace meta {
namespace test {

static_assert( all_match< ::std::is_fundamental, int, float, double, long >::value, "" );
static_assert( !all_match< ::std::is_integral, int, float, double, long >::value, "" );

}  /* namespace test */

}  /* namespace meta */
namespace def {
namespace test {

struct eventAB{};
struct eventBC{};
struct eventAC{};

struct stateA : state< stateA > {};
struct stateB : state< stateB > {};
struct stateC : state< stateC > {};


using transition_table_1 = transition_table<
            transition<stateA, eventAB, stateB>,
            transition<stateA, eventAC, stateC>,
            transition<stateB, eventBC, stateC>
        >;

struct my_state : state <my_state> {
    using internal_transitions = transition_table <
        internal_transition< eventAB >,
        internal_transition< eventBC >
    >;
};
struct my_stable_state : state< my_stable_state > {
};

static_assert( !::std::is_same<my_state::internal_transitions, void>::value, "" );
static_assert(my_state::internal_transitions::transitions::size == 2, "");
static_assert( ::std::is_same<my_stable_state::internal_transitions, void>::value, "" );



TEST(Dummy, Dummy)
{

}

}  /* namespace test */
}  /* namespace def */
}  /* namespace afsm */

