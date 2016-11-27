/*
 * definition.hpp
 *
 *  Created on: 27 мая 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef AFSM_DEFINITION_HPP_
#define AFSM_DEFINITION_HPP_

#include <pushkin/meta.hpp>
#include <afsm/fsm_fwd.hpp>
#include <afsm/definition_fwd.hpp>
#include <afsm/detail/def_traits.hpp>

namespace afsm {
namespace def {

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

template < typename SourceState, typename Event, typename Guard >
struct transition_key {
    using source_state_type     = SourceState;
    using event_type            = Event;
    using guard_type            = Guard;
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

    using key_type  = detail::transition_key<SourceState, Event, Guard>;

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

    using key_type  = detail::transition_key<void, Event, Guard>;

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
                ::psst::meta::all_match< traits::is_transition, T ... >,
                ::std::true_type
            >::type::value,
            "Transition table can contain only transition or internal_transition template instantiations" );
    using transitions = ::psst::meta::type_tuple<T...>;
    using transition_map = ::psst::meta::type_map<
            ::psst::meta::type_tuple<typename T::key_type ...>,
            ::psst::meta::type_tuple<typename T::value_type...>>;
    using inner_states  = typename ::psst::meta::unique<
                typename detail::source_state<T>::type ...,
                typename detail::target_state<T>::type ...
            >::type;
    using handled_events = typename ::psst::meta::unique<
            typename T::event_type ... >::type;

    static constexpr ::std::size_t size                 = transition_map::size;
    static constexpr ::std::size_t transition_count     = transition_map::size;
    static constexpr ::std::size_t inner_state_count    = inner_states::size;
    static constexpr ::std::size_t event_count          = handled_events::size;

    static_assert(
            ::std::conditional<
                 (inner_state_count > 0),
                 ::psst::meta::all_match< traits::is_state, inner_states >,
                 ::std::true_type
            >::type::value,
            "State types must derive from afsm::def::state");
    static_assert(transitions::size == transition_count, "Duplicate transition");
    // TODO Check for different guards for transitions from one state on one event
};

template < typename StateType, typename ... Tags >
struct state : tags::state, Tags... {
    using state_type            = StateType;
    using base_state_type       = state<state_type, Tags...>;
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
    using not_ = ::psst::meta::not_<Predicate>;
    template < typename ... Predicates >
    using and_ = ::psst::meta::and_<Predicates...>;
    template < typename ... Predicates >
    using or_ = ::psst::meta::or_<Predicates...>;
    template < typename ... T >
    using type_tuple = ::psst::meta::type_tuple<T...>;
};

template < typename StateType, typename ... Tags >
struct terminal_state : tags::state, Tags... {
    using state_type            = StateType;
    using base_state_type       = terminal_state<state_type, Tags ...>;
    using internal_transitions  = void;
    using transitions           = void;
    using deferred_events       = void;
    using activity              = void;
};

template < typename StateMachine, typename ... Tags >
struct state_machine : state< StateMachine, Tags... >, tags::state_machine {
    using state_machine_type    = StateMachine;
    using base_state_type       = state_machine< state_machine_type, Tags... >;
    using initial_state         = void;
    using internal_transitions  = void;
    using transitions           = void;
    using deferred_events       = void;
    using activity              = void;
    using orthogonal_regions    = void;

    template <typename SourceState, typename Event, typename TargetState,
            typename Action = none, typename Guard = none>
    using tr = transition<SourceState, Event, TargetState, Action, Guard>;

    using inner_states_definition = traits::inner_states_definitions<state<StateMachine, Tags...>>;

    template < typename T, typename ... TTags>
    using state = typename inner_states_definition::template state<T, TTags...>;
    template < typename T, typename ... TTags >
    using terminal_state = typename inner_states_definition::template terminal_state<T, TTags ...>;
    template < typename T, typename ... TTags >
    using inner_machine = typename inner_states_definition::template state_machine<T, TTags...>;

    using none = afsm::none;
    template < typename Predicate >
    using not_ = ::psst::meta::not_<Predicate>;
    template < typename ... Predicates >
    using and_ = ::psst::meta::and_<Predicates...>;
    template < typename ... Predicates >
    using or_ = ::psst::meta::or_<Predicates...>;
    template < typename ... T >
    using type_tuple = ::psst::meta::type_tuple<T...>;
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
    using type = ::psst::meta::type_tuple<>;
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
          traits::is_state_machine<T>::value,
          handled_events<state_machine<T>>,
          handled_events<state<T>>
      >::type {};

template < typename ... T >
struct handled_events< transition_table<T...> > {
    using type = typename transition_table<T...>::handled_events;
};

template <>
struct handled_events<void> {
    using type = ::psst::meta::type_tuple<>;
};

template < typename T >
struct handled_events< state<T> > {
    using type = typename handled_events<
            typename T::internal_transitions >::type;
};

template < typename T >
struct handled_events< state_machine<T> > {
    using type = typename ::psst::meta::unique<
                typename handled_events< typename T::internal_transitions >::type,
                typename handled_events< typename T::transitions >::type
            >::type;
};

/**
 * Events handled by a set of states
 */
template < typename ... T >
struct handled_events< ::psst::meta::type_tuple<T...> > {
    using type = typename ::psst::meta::unique<
                typename handled_events<T>::type ...
            >::type;
};

template < typename T >
struct recursive_handled_events
    : ::std::conditional<
        traits::is_state_machine<T>::value,
        recursive_handled_events< state_machine<T> >,
        handled_events< state<T> >
    >::type {};

template < typename ... T >
struct recursive_handled_events< transition_table<T...> > {
    using type = typename ::psst::meta::unique<
            typename transition_table<T...>::handled_events,
            typename recursive_handled_events<
                typename transition_table<T...>::inner_states >::type
        >::type;
};

template <>
struct recursive_handled_events<void> {
    using type = ::psst::meta::type_tuple<>;
};

template < typename T >
struct recursive_handled_events< state_machine<T> > {
    using type = typename ::psst::meta::unique<
                typename handled_events< typename T::internal_transitions >::type,
                typename recursive_handled_events< typename T::transitions >::type
            >::type;
};

template < typename T, typename ... Y >
struct recursive_handled_events< ::psst::meta::type_tuple<T, Y...> > {
    using type = typename ::psst::meta::unique<
                typename recursive_handled_events<T>::type,
                typename recursive_handled_events< ::psst::meta::type_tuple<Y...>>::type
            >::type;
};

template < typename T >
struct recursive_handled_events< ::psst::meta::type_tuple<T> >
    : recursive_handled_events<T> {};

template <>
struct recursive_handled_events< ::psst::meta::type_tuple<> > {
    using type = ::psst::meta::type_tuple<>;
};

template < typename T >
struct has_default_transitions;

template < typename ... T >
struct has_default_transitions< ::psst::meta::type_tuple<T...> >
    : ::psst::meta::contains< none, ::psst::meta::type_tuple<T...> > {};

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
template < typename T, typename SubState >
struct contains_substate;

namespace detail {

template < typename T, typename SubState >
struct contains_recursively :
    ::std::conditional<
        ::std::is_same<T, SubState>::value,
        ::std::true_type,
        typename def::contains_substate<T, SubState>::type
    >::type {};

template < typename SubState >
struct contains_predicate {
    template < typename T >
    using type = contains_recursively<T, SubState>;
};

template < typename T, typename SubState, bool IsMachine >
struct contains_substate : ::std::false_type {};

template < typename T, typename SubState >
struct contains_substate< T, SubState, true >
    : ::std::conditional<
        ::psst::meta::any_match<
            contains_predicate<SubState>::template type,
            typename inner_states< typename T::transitions >::type >::value,
        ::std::true_type,
        ::std::false_type
    >::type {};

}  /* namespace detail */

template < typename T, typename SubState >
struct contains_substate :
    detail::contains_substate<T, SubState, traits::is_state_machine<T>::value> {};

template < typename SubState >
struct contains_substate_predicate {
    template < typename T >
    using type = contains_substate<T, SubState>;
};

}  /* namespace def */
}  /* namespace afsm */


#endif /* AFSM_DEFINITION_HPP_ */
