/*
 * fsm.hpp
 *
 *  Created on: 25 мая 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef AFSM_FSM_HPP_
#define AFSM_FSM_HPP_

#include <type_traits>
#include <afsm/meta.hpp>

namespace afsm {

struct none {};

namespace def {
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

template < typename T >
struct is_transition : ::std::false_type {};

template < typename SourceState, typename Event, typename TargetState,
    typename Action, typename Guard>
struct is_transition<transition<SourceState, Event, TargetState, Action, Guard>>
    : ::std::true_type{};
template < typename Event, typename Action, typename Guard >
struct is_transition< internal_transition< Event, Action, Guard > >
    : ::std::true_type {};

template < typename T >
struct is_internal_transition;

template < typename SourceState, typename Event, typename TargetState,
    typename Action, typename Guard>
struct is_internal_transition<transition<SourceState, Event, TargetState, Action, Guard>>
    : ::std::false_type{};
template < typename Event, typename Action, typename Guard >
struct is_internal_transition< internal_transition< Event, Action, Guard > >
    : ::std::true_type {};
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
    static_assert( meta::all_match< detail::is_transition, T ... >::value,
            "Transition table can contain only transition or internal_transition template instantiations" );
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
};

}  /* namespace def */

}  /* namespace afsm */

#endif /* AFSM_FSM_HPP_ */
