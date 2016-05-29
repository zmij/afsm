/*
 * definition.hpp
 *
 *  Created on: 27 мая 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef AFSM_DEFINITION_HPP_
#define AFSM_DEFINITION_HPP_

#include <afsm/meta.hpp>
#include <afsm/fsm_fwd.hpp>

namespace afsm {

namespace def {
template < typename SourceState, typename Event, typename TargetState,
        typename Action = none, typename Guard = none >
struct transition;
template < typename Event, typename Action = none, typename Guard = none >
struct internal_transition;
template < typename ... T >
struct transition_table;
template < typename T >
struct state;
template < typename T >
struct state_machine;

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

template <typename T>
struct event_type;

template < typename SourceState, typename Event, typename TargetState,
    typename Action, typename Guard>
struct event_type< transition<SourceState, Event, TargetState, Action, Guard> > {
    using type = Event;
};
template < typename Event, typename Action, typename Guard >
struct event_type< internal_transition< Event, Action, Guard > > {
    using type = Event;
};


template < typename T >
struct is_transition : ::std::false_type {};

template < typename SourceState, typename Event, typename TargetState,
    typename Action, typename Guard>
struct is_transition< transition<SourceState, Event, TargetState, Action, Guard> >
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

template < typename T >
struct is_state : ::std::conditional<
        ::std::is_base_of< state<T>, T >::value,
        ::std::true_type,
        ::std::false_type
    >::type {};

template < typename T >
struct is_state_machine : ::std::conditional<
        ::std::is_base_of< state_machine<T>, T >::value,
        ::std::true_type,
        ::std::false_type
    >::type {};

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

    static_assert(!::std::is_same<Event, none>::value,
            "Internal transition must have a trigger");
};

template < typename ... T >
struct transition_table {
    static_assert(
            ::std::conditional<
                (sizeof ... (T) > 0),
                meta::all_match< detail::is_transition, T ... >,
                ::std::true_type
            >::type::value,
            "Transition table can contain only transition or internal_transition template instantiations" );
    using transitions = meta::type_tuple<T...>;
    using transition_map = meta::type_map<
            meta::type_tuple<typename T::key_type ...>,
            meta::type_tuple<typename T::value_type...>>;
    using inner_states  = typename meta::type_set<
                typename detail::source_state<T>::type ...,
                typename detail::target_state<T>::type ...
            >::type;
    using handled_events = typename meta::type_set<
            typename T::event_type ... >::type;

    static constexpr ::std::size_t transition_count     = transition_map::size;
    static constexpr ::std::size_t inner_state_count    = inner_states::size;
    static constexpr ::std::size_t event_count          = handled_events::size;

    static_assert(
            ::std::conditional<
                 (inner_state_count > 0),
                 meta::all_match< detail::is_state, inner_states >,
                 ::std::true_type
            >::type::value,
            "State types must derive from afsm::def::state");
    static_assert(transitions::size == transition_count, "Duplicate transition");

};

template < typename StateType >
struct state {
    using state_type            = StateType;
    using base_type             = state<state_type>;
    using internal_transitions  = void;
    using transitions           = void;
    using deferred_events       = void;

    template < typename Event, typename Action, typename Guard >
    using in = internal_transition< Event, Action, Guard >;
};

template < typename StateMachine >
struct state_machine : state< StateMachine >{
    using state_machine_type    = StateMachine;
    using base_type             = state_machine< state_machine_type >;
    using initial_state         = void;

    template <typename SourceState, typename Event, typename TargetState,
            typename Action = none, typename Guard = none>
    using tr = transition<SourceState, Event, TargetState, Action, Guard>;
};

namespace detail {

template < typename T >
struct has_inner_states : ::std::false_type {};
template < typename ... T >
struct has_inner_states< transition_table<T...> >
    : ::std::integral_constant<bool,
        (transition_table<T...>::inner_state_count > 0)> {};

template < typename T >
struct inner_states {
    using type = meta::type_tuple<>;
};

template < typename ... T >
struct inner_states< transition_table<T...> > {
    using type = typename transition_table<T...>::inner_states;
};

template < typename T >
struct has_transitions : ::std::false_type {};
template < typename ... T >
struct has_transitions< transition_table<T...> >
    : ::std::conditional<
        (transition_table<T...>::transition_count > 0),
        ::std::true_type,
        ::std::false_type
    >::type{};

template < typename T >
struct handled_events
    : std::conditional<
          is_state_machine<T>::value,
          handled_events<state_machine<T>>,
          handled_events<state<T>>
      >::type {};

template < typename ... T >
struct handled_events< transition_table<T...> > {
    using type = typename transition_table<T...>::handled_events;
};

template <>
struct handled_events<void> {
    using type = meta::type_tuple<>;
};

template < typename T >
struct handled_events< state<T> > {
    using type = typename handled_events<
            typename T::internal_transitions >::type;
};

template < typename T >
struct handled_events< state_machine<T> > {
    using type = typename meta::type_set<
                typename handled_events< typename T::internal_transitions >::type,
                typename handled_events< typename T::transitions >::type
            >::type;
};

/**
 * Events handled by a set of states
 */
template < typename ... T >
struct handled_events< meta::type_tuple<T...> > {
    using type = typename meta::type_set<
                typename handled_events<T>::type ...
            >::type;
};

template < typename T >
struct recursive_handled_events
    : ::std::conditional<
        is_state_machine<T>::value,
        recursive_handled_events< state_machine<T> >,
        handled_events< state<T> >
    >::type {};

template < typename ... T >
struct recursive_handled_events< transition_table<T...> > {
    using type = typename meta::type_set<
            typename transition_table<T...>::handled_events,
            typename recursive_handled_events<
                typename transition_table<T...>::inner_states >::type
        >::type;
};

template <>
struct recursive_handled_events<void> {
    using type = meta::type_tuple<>;
};

template < typename T >
struct recursive_handled_events< state_machine<T> > {
    using type = typename meta::type_set<
                typename handled_events< typename T::internal_transitions >::type,
                typename recursive_handled_events< typename T::transitions >::type
            >::type;
};

template < typename T, typename ... Y >
struct recursive_handled_events< meta::type_tuple<T, Y...> > {
    using type = typename meta::type_set<
                typename recursive_handled_events<T>::type,
                typename recursive_handled_events< meta::type_tuple<Y...>>::type
            >::type;
};

template < typename T >
struct recursive_handled_events< meta::type_tuple<T> >
    : recursive_handled_events<T> {};

template <>
struct recursive_handled_events< meta::type_tuple<> > {
    using type = meta::type_tuple<>;
};

template < typename T >
struct has_default_transitions;

template < typename ... T >
struct has_default_transitions< meta::type_tuple<T...> >
    : meta::contains< none, meta::type_tuple<T...> > {};

template < typename T >
struct is_default_transition
    : ::std::conditional<
        ::std::is_same< typename T::event_type, none >::value,
        ::std::true_type,
        ::std::false_type
    >::type {};

template < typename T >
struct is_unguarded_transition
    : ::std::conditional<
        ::std::is_same< typename T::guard_type, none >::value,
        ::std::true_type,
        ::std::false_type
    >::type {};

template < typename T >
struct is_default_unguarded_transition
    : ::std::conditional<
        is_default_transition<T>::value && is_unguarded_transition<T>::value,
        ::std::true_type,
        ::std::false_type
    >::type {};

}  /* namespace detail */
}  /* namespace def */
}  /* namespace afsm */


#endif /* AFSM_DEFINITION_HPP_ */