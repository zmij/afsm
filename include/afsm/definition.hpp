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
template < typename T, bool History = false, typename CommonBase = void >
struct state;
template < typename T, typename CommonBase = void  >
struct terminal_state;

template < typename T, bool History = false, typename CommonBase = void >
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
struct is_internal_transition : ::std::false_type {};

template < typename Event, typename Action, typename Guard >
struct is_internal_transition< internal_transition< Event, Action, Guard > >
    : ::std::true_type {};

template < typename T >
struct is_state : ::std::conditional<
        ::std::is_base_of< state<T, true>, T >::value
         || ::std::is_base_of< state<T, false>, T >::value
         || ::std::is_base_of< terminal_state<T>, T >::value,
        ::std::true_type,
        ::std::false_type
    >::type {};

template < typename T >
struct is_terminal_state : ::std::conditional<
        ::std::is_base_of< terminal_state<T>, T >::value,
        ::std::true_type,
        ::std::false_type
    >::type {};

template <typename T>
struct state_machine_trait {};

template < typename T >
struct is_state_machine : ::std::conditional<
        ::std::is_base_of< state_machine_trait<T>, T >::value,
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

    struct key_type    {
        using transition_type   = transition;
        using event_type        = transition::event_type;
        using guard_type        = transition::guard_type;
        using source_state_type = transition::source_state_type;
    };
    struct value_type    {
        using action_type       = transition::action_type;
        using target_state_type = transition::target_state_type;
    };
};

template < typename Event, typename Action, typename Guard >
struct internal_transition {
    using event_type            = Event;
    using action_type           = Action;
    using guard_type            = Guard;

    struct key_type    {
        using transition_type   = internal_transition;
        using event_type        = internal_transition::event_type;
        using guard_type        = internal_transition::guard_type;
    };
    struct value_type    {
        using action_type       = internal_transition::action_type;
    };

    static_assert(!::std::is_same<Event, none>::value,
            "Internal transition must have a trigger");
};

template < typename Event >
struct handles_event {
    template < typename Transition >
    struct type : ::std::conditional<
        ::std::is_same< typename Transition::event_type, Event >::value,
        ::std::true_type,
        ::std::false_type
    >::type {};
};

template < typename State >
struct originates_from {
    template < typename Transition >
    struct type : ::std::conditional<
        ::std::is_same< typename Transition::source_state_type, State >::value,
        ::std::true_type,
        ::std::false_type
    >::type {};
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

    static constexpr ::std::size_t size                 = transition_map::size;
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
    // TODO Check for different guards for transitions from one state on one event
};

template < typename StateType, bool HasHistory >
struct state< StateType, HasHistory, void > {
    static constexpr bool has_history = HasHistory;

    using state_type            = StateType;
    using base_state_type       = state<state_type, has_history, void>;
    using internal_transitions  = void;
    using transitions           = void;
    using deferred_events       = void;
    using activity              = void;

    template < typename Event, typename Action = none, typename Guard = none >
    using in = internal_transition< Event, Action, Guard>;
    template < typename ... T >
    using transition_table = def::transition_table<T...>;

    using none = afsm::none;
    template < typename Predicate >
    using not_ = meta::not_<Predicate>;
};

template < typename StateType, bool HasHistory, typename CommonBase >
struct state : state<StateType, HasHistory, void>, CommonBase {
    static constexpr bool has_history = HasHistory;
    using state_type            = StateType;
    using base_state_type       = state<state_type, has_history, CommonBase>;
    using internal_transitions  = void;
    using transitions           = void;
    using deferred_events       = void;
    using activity              = void;
};

template < typename StateType >
struct terminal_state< StateType, void > {
    using state_type            = StateType;
    using base_state_type       = terminal_state<state_type, void>;
    using internal_transitions  = void;
    using transitions           = void;
    using deferred_events       = void;
    using activity              = void;
};

template < typename StateType, typename CommonBase >
struct terminal_state : terminal_state<StateType, void>, CommonBase {
    using state_type            = StateType;
    using base_state_type       = terminal_state<state_type, void>;
};

template < typename StateMachine, bool HasHistory, typename CommonBase >
struct state_machine : state< StateMachine, HasHistory, CommonBase >,
        detail::state_machine_trait<StateMachine> {
    static constexpr bool has_history = HasHistory;
    using state_machine_type    = StateMachine;
    using base_state_type       = state_machine< state_machine_type, has_history, CommonBase >;
    using initial_state         = void;
    using internal_transitions  = void;
    using transitions           = void;
    using deferred_events       = void;
    using activity              = void;

    template <typename SourceState, typename Event, typename TargetState,
            typename Action = none, typename Guard = none>
    using tr = transition<SourceState, Event, TargetState, Action, Guard>;

    template < typename T, bool History = false, typename Base = CommonBase >
    using state = def::state<T, History, Base>;

    template < typename T, typename Base = CommonBase >
    using terminal_state = def::terminal_state<T, Base>;

    using none = afsm::none;
    template < typename Predicate >
    using not_ = meta::not_<Predicate>;
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
