/*
 * transitions.hpp
 *
 *  Created on: 29 мая 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef AFSM_DETAIL_TRANSITIONS_HPP_
#define AFSM_DETAIL_TRANSITIONS_HPP_

#include <afsm/detail/actions.hpp>

namespace afsm {
namespace transitions {

namespace detail {

enum class event_handle_type {
    none,
    transition,
    internal_transition,
    inner_state,
};

template <event_handle_type V>
using handle_event = ::std::integral_constant<event_handle_type, V>;

template < typename FSM, typename Event >
struct event_handle_selector
    : ::std::conditional<
        (::psst::meta::find_if<
            def::handles_event<Event>::template type,
            typename FSM::transitions >::type::size > 0),
        handle_event< event_handle_type::transition >,
        typename ::std::conditional<
              (::psst::meta::find_if<
                      def::handles_event<Event>::template type,
                      typename FSM::internal_transitions >::type::size > 0),
              handle_event< event_handle_type::internal_transition >,
              handle_event< event_handle_type::inner_state >
        >::type
    >::type{};

template <typename SourceState, typename Event, typename Transition>
struct transits_on_event
    : ::std::conditional<
        !def::detail::is_internal_transition<Transition>::value &&
        ::std::is_same< typename Transition::source_state_type, SourceState >::value &&
        ::std::is_same< typename Transition::event_type, Event >::value,
        ::std::true_type,
        ::std::false_type
    >::type {};

template < typename State, typename FSM, typename Event >
struct has_on_exit {
private:
    static FSM&         fsm;
    static Event const& event;

    template < typename U >
    static ::std::true_type
    test( decltype( ::std::declval<U>().on_exit(event, fsm) ) const* );

    template < typename U >
    static ::std::false_type
    test(...);
public:
    static constexpr bool value = decltype( test<State>(nullptr) )::value;
};

template < typename State, typename FSM, typename Event >
struct has_on_enter {
private:
    static FSM&         fsm;
    static Event&       event;

    template < typename U >
    static ::std::true_type
    test( decltype( ::std::declval<U>().on_enter(::std::move(event), fsm) ) const* );

    template < typename U >
    static ::std::false_type
    test(...);
public:
    static constexpr bool value = decltype( test<State>(nullptr) )::value;
};

template < typename Activity, typename FSM, typename State >
struct activity_has_start {
private:
    static FSM&         fsm;
    static State&       state;

    template < typename U >
    static ::std::true_type
    test( decltype( ::std::declval<U>().start(fsm, state) ) const* );

    template < typename U >
    static ::std::false_type
    test(...);
public:
    static constexpr bool value = decltype( test<Activity>(nullptr) )::value;
};

template < typename Activity, typename FSM, typename State >
struct activity_has_stop {
private:
    static FSM&         fsm;
    static State&       state;

    template < typename U >
    static ::std::true_type
    test( decltype( ::std::declval<U>().stop(fsm, state) ) const* );

    template < typename U >
    static ::std::false_type
    test(...);
public:
    static constexpr bool value = decltype( test<Activity>(nullptr) )::value;
};

template < typename Activity, typename FSM, typename State >
struct is_valid_activity {
    static constexpr bool value = activity_has_start<Activity, FSM, State>::value
            && activity_has_stop<Activity, FSM, State>::value;
};

template < typename FSM, typename State >
struct is_valid_activity < void, FSM, State >{
    static constexpr bool value = true;
};

template < typename FSM, typename State, typename Event, bool hasExit >
struct state_exit_impl {
    void
    operator()(State& state, Event const& event, FSM& fsm) const
    { state.on_exit(event, fsm); }
};

template < typename FSM, typename State, typename Event >
struct state_exit_impl< FSM, State, Event, false > {
    void
    operator()(State&, Event const&, FSM&) const {}
};

template < typename FSM, typename State, typename Event >
struct state_exit : state_exit_impl< FSM, State, Event,
    has_on_exit<State, FSM, Event>::value > {};

template < typename FSM, typename State, typename Event, bool hasExit >
struct state_enter_impl {
    void
    operator()(State& state, Event&& event, FSM& fsm) const
    { state.on_enter(::std::forward<Event>(event), fsm); }
};

template < typename FSM, typename State, typename Event >
struct state_enter_impl< FSM, State, Event, false > {
    void
    operator()(State&, Event&&, FSM&) const {}
};

template < typename FSM, typename State, typename Event >
struct state_enter : state_enter_impl<FSM, State, Event,
        has_on_enter<State, FSM, Event>::value> {};

template < typename FSM, typename State, bool HasHistory >
struct state_clear_impl {
    void
    operator()(FSM& fsm, State& state) const
    {
        state = typename afsm::detail::front_state_type<FSM, State>::type{fsm};
    }
};

template < typename FSM, typename State >
struct state_clear_impl< FSM, State, true > {
    void
    operator()(FSM&, State&) const
    {}
};

template < typename FSM, typename State >
struct state_clear : state_clear_impl< FSM, State, State::has_history > {};

template < typename FSM, typename StateTable, typename Event >
struct no_transition {
    explicit
    no_transition(StateTable&) {}

    actions::event_process_result
    operator()(Event&&) const
    {
        return actions::event_process_result::refuse;
    }
};

template < typename FSM, typename StateTable, typename Transition >
struct single_transition;

template < typename FSM, typename StateTable,
        typename SourceState, typename Event, typename TargetState,
        typename Action, typename Guard >
struct single_transition<FSM, StateTable,
    ::psst::meta::type_tuple< def::transition<SourceState, Event, TargetState, Action, Guard> > > {

    using fsm_type          = FSM;
    using state_table       = StateTable;
    using source_state_type = SourceState;
    using target_state_type = TargetState;

    using states_def        = typename fsm_type::inner_states_def;

    using guard_type        = actions::detail::guard_check<FSM, SourceState, Guard>;
    using source_exit       = state_exit<fsm_type, source_state_type, Event>;
    using target_enter      = state_enter<fsm_type, target_state_type, Event>;
    using action_type       = actions::detail::action_invokation<Action, FSM,
            Event, SourceState, TargetState>;
    using state_clear_type  = state_clear<FSM, SourceState>;

    using source_index = ::psst::meta::index_of<source_state_type, states_def>;
    using target_index = ::psst::meta::index_of<target_state_type, states_def>;

    static_assert(source_index::found, "Failed to find source state index");
    static_assert(target_index::found, "Failed to find target state index");

    state_table&    states;

    explicit
    single_transition(state_table& states_)
        : states{states_} {}

    actions::event_process_result
    operator()(Event&& event) const
    {
        return states.template transit_state< source_state_type, target_state_type >
            ( ::std::forward<Event>(event), guard_type{}, action_type{},
                    source_exit{}, target_enter{}, state_clear_type{});
    }
};

template < ::std::size_t N, typename FSM, typename StateTable, typename Transitions >
struct nth_transition {
    static_assert(Transitions::size > N, "Transition list is too small");
    using transition            = typename Transitions::template type<N>;
    using event_type            = typename transition::event_type;
    using previous_transition   = nth_transition<N - 1, FSM, StateTable, Transitions>;
    using transition_invokation = single_transition<FSM, StateTable, ::psst::meta::type_tuple<transition>>;

    transition_invokation action;

    explicit
    nth_transition(StateTable& states)
        : action{states} {}

    actions::event_process_result
    operator()(event_type&& event) const
    {
        auto res = previous_transition{ action.states }
                    (::std::forward<event_type>(event));
        if (res == actions::event_process_result::refuse) {
            return action(::std::forward<event_type>(event));
        }
        return res;
    }
};

template < typename FSM, typename StateTable, typename Transitions >
struct nth_transition< 0, FSM, StateTable, Transitions > {
    static_assert(Transitions::size > 0, "Transition list is too small");
    using transition            = typename Transitions::template type<0>;
    using event_type            = typename transition::event_type;
    using transition_invokation = single_transition<FSM, StateTable, ::psst::meta::type_tuple<transition>>;

    transition_invokation action;

    explicit
    nth_transition(StateTable& states)
        : action{states} {}

    actions::event_process_result
    operator()(event_type&& event) const
    {
        return action(::std::forward<event_type>(event));
    }
};

template < typename FSM, typename StateTable, typename Event, typename Transitions >
struct conditional_transition {
    static constexpr ::std::size_t size = Transitions::size;
    static_assert(Transitions::size > 0, "Transition list is too small");

    using last_transition = nth_transition<size - 1, FSM, StateTable, Transitions>;

    StateTable&     states;

    explicit
    conditional_transition(StateTable& states_)
        : states{states_} {}

    actions::event_process_result
    operator()(Event&& event) const
    {
        return last_transition{states}(::std::forward<Event>(event));
    }
};

template < typename FSM, typename StateTable, typename Event, typename Transitions >
struct transition_action_selector {
    using type = typename ::std::conditional<
            Transitions::size == 0,
            no_transition<FSM, StateTable, Event>,
            typename ::std::conditional<
                Transitions::size == 1,
                single_transition<FSM, StateTable, Transitions>,
                conditional_transition<FSM, StateTable, Event, Transitions>
            >::type
        >::type;
};

}  /* namespace detail */

template < typename FSM, typename FSM_DEF, typename Size >
class state_transition_table {
public:
    using fsm_type                      = FSM;
    using state_machine_definition_type = FSM_DEF;
    using size_type                     = Size;

    using this_type         = state_transition_table<FSM, FSM_DEF, Size>;

    using transitions       =
            typename state_machine_definition_type::transitions;
    using transitions_tuple =
            typename transitions::transitions;
    using initial_state     =
            typename state_machine_definition_type::initial_state;
    using inner_states_def  =
            typename def::detail::inner_states< transitions >::type;
    using inner_states_constructor =
            afsm::detail::front_state_tuple< fsm_type, inner_states_def >;
    using inner_states_tuple =
            typename inner_states_constructor::type;

    static constexpr ::std::size_t initial_state_index =
            ::psst::meta::index_of<initial_state, inner_states_def>::value;
    static constexpr ::std::size_t size = inner_states_def::size;

    using state_indexes     = typename ::psst::meta::index_builder<size>::type;

    template < typename Event >
    using invokation_table = ::std::array<
            ::std::function< actions::event_process_result(Event&&) >, size >;
public:
    state_transition_table(fsm_type& fsm)
        : fsm_{fsm},
          current_state_{initial_state_index},
          states_{ inner_states_constructor::construct(fsm) }
    {}

    inner_states_tuple&
    states()
    { return states_; }
    inner_states_tuple const&
    states() const
    { return states_; }

    template < ::std::size_t N>
    ::std::tuple_element< N, inner_states_tuple >&
    get_state()
    { return ::std::get<N>(states_); }
    template < ::std::size_t N>
    ::std::tuple_element< N, inner_states_tuple > const&
    get_state() const
    { return ::std::get<N>(states_); }

    ::std::size_t
    current_state() const
    { return (::std::size_t)current_state_; }

    void
    set_current_state(::std::size_t val)
    { current_state_ = val; }

    template < typename Event >
    actions::event_process_result
    process_event(Event&& event)
    {
        auto inv_table = state_table<Event>( state_indexes{} );
        return inv_table[current_state()](::std::forward<Event>(event));
    }

    template < typename SourceState, typename TargetState,
        typename Event, typename Guard, typename Action,
        typename SourceExit, typename TargetEnter, typename SourceClear >
    actions::event_process_result
    transit_state(Event&& event, Guard guard, Action action, SourceExit exit,
            TargetEnter enter, SourceClear clear)
    {
        using source_index = ::psst::meta::index_of<SourceState, inner_states_def>;
        using target_index = ::psst::meta::index_of<TargetState, inner_states_def>;

        static_assert(source_index::found, "Failed to find source state index");
        static_assert(target_index::found, "Failed to find target state index");

        auto source = ::std::get< source_index::value >(states_);
        auto target = ::std::get< target_index::value >(states_);
        try {
            if (guard(fsm_, source)) {
                exit(source, ::std::forward<Event>(event), fsm_);
                action(::std::forward<Event>(event), fsm_, source, target);
                enter(target, ::std::forward<Event>(event), fsm_);
                clear(fsm_, source);
                current_state_ = target_index::value;
                return actions::event_process_result::process;
            }
        } catch (...) {
            // FIXME Do something ;)
            throw;
        }
        return actions::event_process_result::refuse;
    }
private:
    template < typename Event, ::std::size_t ... Indexes >
    invokation_table< Event >
    state_table( ::psst::meta::indexes_tuple< Indexes... > const& )
    {
        using event_transitions = typename ::psst::meta::find_if<
                def::handles_event<Event>::template type, transitions_tuple >::type;
        return invokation_table< Event > {{
            typename detail::transition_action_selector< fsm_type, this_type, Event,
                typename ::psst::meta::find_if<
                    def::originates_from<
                        typename inner_states_def::template type< Indexes >
                    >::template type,
                    event_transitions
                >::type >::type( *this ) ...
        }};
    }
private:
    fsm_type&           fsm_;
    size_type           current_state_;
    inner_states_tuple  states_;
};

}  /* namespace transitions */
}  /* namespace afsm */

#endif /* AFSM_DETAIL_TRANSITIONS_HPP_ */
