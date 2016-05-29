/*
 * actions.hpp
 *
 *  Created on: 28 мая 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef AFSM_DETAIL_ACTIONS_HPP_
#define AFSM_DETAIL_ACTIONS_HPP_

#include <afsm/definition.hpp>
#include <afsm/detail/base_states.hpp>

namespace afsm {
namespace actions {

namespace detail {

template <typename TransitionKey >
struct is_internal_transition : ::std::false_type {};

template < typename Event, typename Action, typename Guard >
struct is_internal_transition<
    typename def::internal_transition<Event, Action, Guard> > : ::std::true_type {};

template <typename Event, typename Transition>
struct handles_event
    : ::std::conditional<
        ::std::is_same< typename Transition::event_type, Event >::value,
        ::std::true_type,
        ::std::false_type
    >::type {};

template < typename FSM, typename State, typename Guard >
struct guard_check {
    static_assert(meta::is_callable< Guard, FSM, State >::value,
            "Guard object is not callable with needed parameters");

    bool
    operator()(FSM const& fsm, State const& state) const
    { return Guard{}(fsm, state); }
};

template < typename FSM, typename State >
struct guard_check< FSM, State, none > {
    bool
    operator()(FSM const& fsm, State const& state) const
    { return true; }
};

template < typename Action, typename FSM, typename Event,
    typename SourceState, typename TargetState >
struct action_invokation {
    static_assert(meta::is_callable< Action, Event&&,
                FSM&, SourceState&, TargetState& >::value,
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
    operator()(Event&& event, FSM& fsm, SourceState& source, TargetState& target) const
    {}
};

template < typename Action, typename Guard, typename FSM,
    typename Event, typename SourceState, typename TargetState >
struct guarded_action_invokation {
    using guard_type        = guard_check< FSM, SourceState, Guard >;
    using invokation_type   = action_invokation< Action, FSM, Event, SourceState, TargetState >;

    bool
    operator()(Event&& event, FSM& fsm, SourceState& source, TargetState& target) const
    {
        if (guard_type{}(fsm, source)) {
            invokation_type{}(::std::forward<Event>(event), fsm, source, target);
            return true;
        }
        return false;
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

    bool
    operator()(Event&& event, FSM& fsm, State& state) const
    {
        if (!previous_action{}(::std::forward<Event>(event), fsm, state))
            return invokation_type{}(::std::forward<Event>(event), fsm, state, state);
        return true;
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

    bool
    operator()(Event&& event, FSM& fsm, State& state) const
    {
        return invokation_type{}(::std::forward<Event>(event), fsm, state, state);
    }
};

template < typename FSM, typename State, typename Event, typename Transition >
struct unconditional_in_state_invokation {
    using action_type = typename Transition::action_type;
    using guard_type  = typename Transition::guard_type;
    using invokation_type = guarded_action_invokation<
                action_type, guard_type, FSM, Event, State, State >;

    void
    operator()(Event&& event, FSM& fsm, State& state) const
    {
        invokation_type{}(::std::forward<Event>(event), fsm, state, state);
    }
};

template < typename FSM, typename State, typename Event,
        typename Transitions >
struct conditional_in_state_invokation {
    static constexpr ::std::size_t size = Transitions::size;
    using invokation_type = nth_inner_invokation< size - 1, FSM, State, Event, Transitions>;

    void
    operator()(Event&& event, FSM& fsm, State& state) const
    {
        invokation_type{}(::std::forward<Event>(event), fsm, state);
    }
};

}  /* namespace detail */

template < typename FSM, typename State, typename Event >
struct in_state_action_invokation {
    using fsm_type          = FSM;
    using state_type        = State;
    using event_type        = Event;

    using transitions       = typename state_type::internal_transitions;
    static_assert( !::std::is_same<transitions, void>::value,
            "State doesn't have internal transitions table" );

    template < typename Transition >
    using handles_event     = detail::handles_event< event_type, Transition >;
    using event_handlers    = typename meta::find_if<
        handles_event, typename transitions::transitions >::type;
    static_assert( event_handlers::size > 0, "State doesn't handle event" );
    using handler_type      = typename ::std::conditional<
                event_handlers::size == 1,
                detail::unconditional_in_state_invokation<
                    fsm_type, state_type, event_type,
                    typename event_handlers::template type<0>>,
                detail::conditional_in_state_invokation<
                    fsm_type, state_type, event_type, event_handlers>
            >::type;

    void
    operator()(event_type&& event, fsm_type& fsm, state_type& state) const
    {
       handler_type{}(::std::forward<event_type>(event), fsm, state);
    }
};

template < typename FSM, typename State, typename Event >
void
handle_in_state_event(Event&& event, FSM& fsm, State& state)
{
    in_state_action_invokation< FSM, State, Event >{}
        (::std::forward<Event>(event), fsm, state);
}

}  /* namespace actions */
}  /* namespace afsm */

#endif /* AFSM_DETAIL_ACTIONS_HPP_ */
