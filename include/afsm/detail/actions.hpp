/*
 * actions.hpp
 *
 *  Created on: 28 мая 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef AFSM_DETAIL_ACTIONS_HPP_
#define AFSM_DETAIL_ACTIONS_HPP_

#include <afsm/definition.hpp>
#include <functional>
#include <array>

namespace afsm {
namespace actions {

enum class event_process_result {
    refuse,
    process,
    process_in_state,    /**< Process with in-state transition */
    defer,
};


namespace detail {

template <typename Action, typename Event, typename FSM,
        typename SourceState, typename TargetState>
struct action_is_callable {
private:
    static FSM&         fsm;
    static Event&       event;
    static SourceState& source;
    static TargetState& target;

    template < typename U >
    static ::std::true_type
    test( decltype( ::std::declval<U>()(::std::move(event), fsm, source, target) ) const* );

    template < typename U >
    static ::std::false_type
    test(...);
public:
    static constexpr bool value = decltype( test<Action>(nullptr) )::value;
};

template <typename Event, typename Transition>
struct handles_event
    : ::std::conditional<
        ::std::is_same< typename Transition::event_type, Event >::value,
        ::std::true_type,
        ::std::false_type
    >::type {};

template < typename FSM, typename State, typename Guard >
struct guard_check {
    static_assert(::psst::meta::is_callable< Guard, FSM const&, State const& >::value,
            "Guard object is not callable with needed parameters");

    bool
    operator()(FSM const& fsm, State const& state) const
    { return Guard{}(fsm, state); }
};

template < typename FSM, typename State >
struct guard_check< FSM, State, none > {
    bool
    operator()(FSM const&, State const&) const
    { return true; }
};

template < typename Action, typename FSM, typename Event,
    typename SourceState, typename TargetState >
struct action_invokation {
    static_assert(action_is_callable< Action, Event,
                FSM, SourceState, TargetState >::value,
            "Action is not callable for this transition");

    void
    operator()(Event&& event, FSM& fsm, SourceState& source, TargetState& target) const
    {
        Action{}(::std::forward<Event>(event), fsm, source, target);
    }
};

template < typename FSM, typename Event,
    typename SourceState, typename TargetState >
struct action_invokation< none, FSM, Event, SourceState, TargetState > {
    void
    operator()(Event&&, FSM&, SourceState&, TargetState&) const
    {}
};

template < typename Action, typename Guard, typename FSM,
    typename Event, typename SourceState, typename TargetState >
struct guarded_action_invokation {
    using guard_type        = guard_check< FSM, SourceState, Guard >;
    using invokation_type   = action_invokation< Action, FSM, Event, SourceState, TargetState >;

    event_process_result
    operator()(Event&& event, FSM& fsm, SourceState& source, TargetState& target) const
    {
        if (guard_type{}(fsm, source)) {
            invokation_type{}(::std::forward<Event>(event), fsm, source, target);
            return event_process_result::process;
        }
        return event_process_result::refuse;
    }
};

template < ::std::size_t N, typename FSM, typename State, typename Event,
        typename Transitions >
struct nth_inner_invokation {
    static_assert(Transitions::size > N, "Transitions list is too small");
    using transition        = typename Transitions::template type<N>;
    using action_type       = typename transition::action_type;
    using guard_type        = typename transition::guard_type;
    using invokation_type   = guarded_action_invokation<
                action_type, guard_type, FSM, Event, State, State >;
    using previous_action    = nth_inner_invokation<N - 1, FSM, State, Event, Transitions>;

    event_process_result
    operator()(Event&& event, FSM& fsm, State& state) const
    {
        auto res = previous_action{}(::std::forward<Event>(event), fsm, state);
        if (res == event_process_result::refuse)
            return invokation_type{}(::std::forward<Event>(event), fsm, state, state);
        return res;
    }
};

template < typename FSM, typename State, typename Event, typename Transitions >
struct nth_inner_invokation<0, FSM, State, Event, Transitions> {
    static_assert(Transitions::size > 0, "Transitions list is empty");
    using transition        = typename Transitions::template type<0>;
    using action_type       = typename transition::action_type;
    using guard_type        = typename transition::guard_type;
    using invokation_type   = guarded_action_invokation<
                action_type, guard_type, FSM, Event, State, State >;

    event_process_result
    operator()(Event&& event, FSM& fsm, State& state) const
    {
        return invokation_type{}(::std::forward<Event>(event), fsm, state, state);
    }
};

struct no_in_state_invokation {
    template < typename FSM, typename State, typename Event >
    event_process_result
    operator()(Event&&, FSM&, State&) const
    {
        return event_process_result::refuse;
    }
};

template < typename FSM, typename State, typename Event, typename Transition >
struct unconditional_in_state_invokation {
    using action_type = typename Transition::action_type;
    using guard_type  = typename Transition::guard_type;
    using invokation_type = guarded_action_invokation<
                action_type, guard_type, FSM, Event, State, State >;

    event_process_result
    operator()(Event&& event, FSM& fsm, State& state) const
    {
        return invokation_type{}(::std::forward<Event>(event), fsm, state, state);
    }
};

template < typename FSM, typename State, typename Event,
        typename Transitions >
struct conditional_in_state_invokation {
    static constexpr ::std::size_t size = Transitions::size;
    using invokation_type = nth_inner_invokation< size - 1, FSM, State, Event, Transitions>;

    event_process_result
    operator()(Event&& event, FSM& fsm, State& state) const
    {
        return invokation_type{}(::std::forward<Event>(event), fsm, state);
    }
};

template < bool hasActions, typename FSM, typename State, typename Event >
struct in_state_action_invokation {
    using fsm_type          = FSM;
    using state_type        = State;
    using event_type        = Event;

    using transitions       = typename state_type::internal_transitions;
    static_assert( !::std::is_same<transitions, void>::value,
            "State doesn't have internal transitions table" );

    using event_handlers    = typename ::psst::meta::find_if<
        def::handles_event<event_type>::template type,
        typename transitions::transitions >::type;
    static_assert( event_handlers::size > 0, "State doesn't handle event" );
    using handler_type      = typename ::std::conditional<
                event_handlers::size == 1,
                detail::unconditional_in_state_invokation<
                    fsm_type, state_type, event_type,
                    typename event_handlers::template type<0>>,
                detail::conditional_in_state_invokation<
                    fsm_type, state_type, event_type, event_handlers>
            >::type;

    event_process_result
    operator()(event_type&& event, fsm_type& fsm, state_type& state) const
    {
       auto res = handler_type{}(::std::forward<event_type>(event), fsm, state);
       if (res == event_process_result::process) {
           return event_process_result::process_in_state;
       }
       return res;
    }
};

template < typename FSM, typename State, typename Event >
struct in_state_action_invokation<false, FSM, State, Event> {
    using fsm_type          = FSM;
    using state_type        = State;
    using event_type        = Event;

    event_process_result
    operator()(event_type&&, fsm_type&, state_type&) const
    {
       return event_process_result::refuse;
    }
};

}  /* namespace detail */

template < typename FSM, typename State, typename Event >
struct in_state_action_invokation :
        detail::in_state_action_invokation<
            !::std::is_same<typename State::internal_transitions, void>::value &&
            ::psst::meta::contains<Event, typename State::internal_events>::value,
            FSM, State, Event > {
};

template < typename FSM, typename State, typename Event >
event_process_result
handle_in_state_event(Event&& event, FSM& fsm, State& state)
{
    return in_state_action_invokation< FSM, State, Event >{}
        (::std::forward<Event>(event), fsm, state);
}

namespace detail {

template < typename State >
struct process_event_handler {
    State& state;
    template < typename Event >
    event_process_result
    operator()(Event&& event) const
    {
        return state.process_event(::std::forward<Event>(event));
    }
};

template < typename T >
class inner_dispatch_table;

template < typename ... T >
class inner_dispatch_table< ::std::tuple<T...> > {
public:
    static constexpr ::std::size_t size = sizeof ... (T);
    using states_tuple      = ::std::tuple<T...>;
    using dispatch_tuple    = ::std::tuple< process_event_handler<T>... >;
    using indexes_tuple     = typename ::psst::meta::index_builder< size >::type;
    template < typename Event >
    using invokation_table  = ::std::array<
            ::std::function< event_process_result(Event&&) >, size >;
public:
    explicit
    inner_dispatch_table( states_tuple& states )
        : inner_dispatch_table( states, indexes_tuple{} ) {}

    template < typename Event >
    event_process_result
    process_event(::std::size_t current_state, Event&& event)
    {
        if (current_state >= size)
            throw ::std::logic_error{ "Invalid current state index" };
        auto inv_table = state_table< Event >(indexes_tuple{});
        return inv_table[current_state](::std::forward<Event>(event));
    }
private:
    template < ::std::size_t ... Indexes >
    inner_dispatch_table( states_tuple& states,
            ::psst::meta::indexes_tuple< Indexes... > const& )
        : states_( process_event_handler<T>{::std::get<Indexes>(states)} ... )
    {}
    template < typename Event, ::std::size_t ... Indexes >
    invokation_table<Event>
    state_table( ::psst::meta::indexes_tuple< Indexes... > const& )
    {
        // TODO Cache it
        return invokation_table<Event> {{ ::std::get<Indexes>(states_)... }};
    }
    dispatch_tuple states_;
};

}  /* namespace detail */

}  /* namespace actions */
}  /* namespace afsm */

#endif /* AFSM_DETAIL_ACTIONS_HPP_ */
