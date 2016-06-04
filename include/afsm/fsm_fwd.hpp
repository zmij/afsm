/*
 * fsm_fwd.hpp
 *
 *  Created on: 28 мая 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef AFSM_FSM_FWD_HPP_
#define AFSM_FSM_FWD_HPP_

namespace afsm {

struct none {};

template < typename FSM, typename T >
class state;
template < typename FSM, typename T >
class inner_state_machine;
template < typename T, typename Mutex = none, typename Observer = none >
class state_machine;

}  /* namespace afsm */

#endif /* AFSM_FSM_FWD_HPP_ */
