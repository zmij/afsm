/*
 * definition_fwd.hpp
 *
 *  Created on: 1 июня 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef AFSM_DEFINITION_FWD_HPP_
#define AFSM_DEFINITION_FWD_HPP_

namespace afsm {
namespace def {

template < typename T, typename ... Tags >
struct state;
template < typename T, typename ... Tags  >
struct terminal_state;

template < typename T, typename ... Tags >
struct state_machine;

template < typename SourceState, typename Event, typename TargetState,
        typename Action = none, typename Guard = none >
struct transition;
template < typename Event, typename Action = none, typename Guard = none >
struct internal_transition;
template < typename ... T >
struct transition_table;

}  /* namespace def */
}  /* namespace afsm */

#endif /* AFSM_DEFINITION_FWD_HPP_ */
