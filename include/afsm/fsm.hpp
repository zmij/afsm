/*
 * fsm.hpp
 *
 *  Created on: 25 мая 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef FSM_HPP_
#define FSM_HPP_

#include <type_traits>
#include "meta.hpp"

namespace afsm {

struct none {};

template < typename SourceState, typename Event, typename TargetState,
        typename Action = none, typename Guard = none >
struct transition;
template < typename Event, typename Action = none, typename Guard = none >
struct internal_transition;

namespace detail {

template <typename T>
struct source_state;

template < typename SourceState, typename Event, typename TargetState,
    typename Action, typename Guard>
struct source_state< transition<SourceState, Event, TargetState, Action, Guard> > {
    using type = SourceState;
};

template < typename Event, typename Action, typename Guard >
struct source_state< internal_transition< Event, Action, Guard > > {
    using type = void;
};

template <typename T>
struct target_state;

template < typename SourceState, typename Event, typename TargetState,
    typename Action, typename Guard>
struct target_state< transition<SourceState, Event, TargetState, Action, Guard> > {
    using type = TargetState;
};

template < typename Event, typename Action, typename Guard >
struct target_state< internal_transition< Event, Action, Guard > > {
    using type = void;
};

}  /* namespace detail */

template < typename SourceState, typename Event, typename TargetState,
        typename Action, typename Guard >
struct transition {
    using source_state_type     = SourceState;
    using target_state_type     = TargetState;
    using event_type            = Event;
    using action_type           = Action;
    using guard_type            = Guard;

    using key_type              = meta::type_tuple<
                                    source_state_type, event_type, guard_type>;
    using value_type            = meta::type_tuple<
                                    action_type, target_state_type >;
};

template < typename Event, typename Action, typename Guard >
struct internal_transition {
    using event_type            = Event;
    using action_type           = Action;
    using guard_type            = Guard;

    using key_type              = meta::type_tuple< event_type, guard_type >;
    using value_type            = action_type;
};

template < typename ... T >
struct transition_table {
    using transitions = meta::type_map<
            meta::type_tuple<typename T::key_type ...>,
            meta::type_tuple<typename T::value_type...>>;
    using inner_states  = typename meta::type_set<
                typename detail::source_state<T>::type ...,
                typename detail::target_state<T>::type ...
            >::type;
    using handled_actions = typename meta::type_set<
            typename T::action_type ... >::type;
};

template < typename StateType >
struct state {
    using state_type = StateType;

    using internal_transitions = void;

 //   using _internal_transitions = typename state_type::internal_transitions;
};

struct eventA{};
struct eventB{};
struct my_state : state <my_state> {
    using internal_transitions = transition_table <
        internal_transition< eventA >,
        internal_transition< eventB >
    >;
};
struct my_stable_state : state< my_stable_state > {
};

static_assert( !::std::is_same<my_state::internal_transitions, void>::value, "" );
static_assert(my_state::internal_transitions::transitions::size == 2, "");
static_assert( ::std::is_same<my_stable_state::internal_transitions, void>::value, "" );

}  /* namespace afsm */

#endif /* FSM_HPP_ */
