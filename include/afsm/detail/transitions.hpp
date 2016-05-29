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
        (meta::find_if<
            def::handles_event<Event>::template type,
            typename FSM::transitions >::type::size > 0),
        handle_event< event_handle_type::transition >,
        typename ::std::conditional<
              (meta::find_if<
                      def::handles_event<Event>::template type,
                      typename FSM::internal_transitions >::type::size > 0),
              handle_event< event_handle_type::internal_transition >,
              handle_event< event_handle_type::inner_state >
        >::type
    >::type{};

template <typename SourceState, typename Event, typename Transition>
struct transits_on_event
    : ::std::conditional<
        !actions::detail::is_internal_transition<Transition>::value &&
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

template < typename FSM, typename State, typename Event >
struct state_exit {
    static_assert(has_on_exit<State, FSM, Event>::value,
            "State doens't have an exit on this event");

    void
    operator()(State& state, Event const& event, FSM& fsm) const
    { state.on_exit(event, fsm); }
};

template < typename FSM, typename State, typename Event >
struct state_enter {
    static_assert(has_on_enter<State, FSM, Event>::value,
            "State doesn't have an enter on this event");

    void
    operator()(State& state, Event&& event, FSM& fsm) const
    { state.on_enter(::std::forward<Event>(event), fsm); }
};
}  /* namespace detail */

template < typename FSM, typename Event, typename SourceState >
struct state_transition {
    using fsm_type          = FSM;
    using source_state_type = SourceState;
    using event_type        = Event;

    using transitions       = typename fsm_type::transitions;
    static_assert(!::std::is_same< transitions, void >::value,
            "State machine doesn't declare transitions");
    template < typename Transition >
    using transits = detail::transits_on_event<source_state_type, event_type, Transition>;
    using event_handlers = typename meta::find_if<
            transits, typename transitions::transitions >::type;
    static_assert( event_handlers::size > 0,
            "State machine cannot transit from this state on this event" );

    void
    operator()(event_type&& event, fsm_type& fsm,
            source_state_type& state) const
    {

    }
};

}  /* namespace transitions */
}  /* namespace afsm */

#endif /* AFSM_DETAIL_TRANSITIONS_HPP_ */
