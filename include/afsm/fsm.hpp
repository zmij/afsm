/*
 * fsm.hpp
 *
 *  Created on: 25 мая 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef AFSM_FSM_HPP_
#define AFSM_FSM_HPP_

#include <afsm/detail/base_states.hpp>

namespace afsm {

template < typename FSM, typename T >
class state : public detail::state_base< T > {
public:
    using enclosing_fsm_type    = FSM;
public:
    state(enclosing_fsm_type& fsm)
        : state::state_type{}, fsm_{fsm}
    {}

    template < typename Event >
    actions::event_process_result
    process_event( Event&& evt )
    {
        return process_event_impl(::std::forward<Event>(evt),
                detail::event_process_selector<
                    Event,
                    typename state::handled_events,
                    typename state::deferred_events>{} );
    }
private:
    template < typename Event >
    actions::event_process_result
    process_event_impl(Event&& evt,
        detail::process_type<actions::event_process_result::process> const&)
    {
        return actions::handle_in_state_event(::std::forward<Event>(evt), fsm_, *this);
    }
    template < typename Event >
    actions::event_process_result
    process_event_impl(Event&& evt,
        detail::process_type<actions::event_process_result::defer> const&)
    {
        return actions::event_process_result::defer;
    }
    template < typename Event >
    actions::event_process_result
    process_event_impl(Event&& evt,
        detail::process_type<actions::event_process_result::refuse> const&)
    {
        return actions::event_process_result::refuse;
    }
private:
    enclosing_fsm_type&    fsm_;
};

template < typename FSM, typename T >
class inner_state_machine : public detail::state_machine_base< T, none > {
public:
    using enclosing_fsm_type    = FSM;
    using base_machine_type     = detail::state_machine_base< T, none >;
public:
    inner_state_machine(enclosing_fsm_type& fsm)
        : inner_state_machine::machine_type{}, fsm_{fsm} {}

    template < typename Event >
    actions::event_process_result
    process_event( Event&& evt )
    {
        return process_event_impl(fsm_, ::std::forward<Event>(evt),
                detail::event_process_selector<
                    Event,
                    typename inner_state_machine::handled_events,
                    typename inner_state_machine::deferred_events>{} );
    }
private:
    using base_machine_type::process_event_impl;
private:
    enclosing_fsm_type&    fsm_;
};

template < typename T, typename Mutex >
class state_machine : public detail::state_machine_base< T, Mutex > {
public:
    static_assert( meta::is_empty< typename state_machine::deferred_events >::value,
            "Outer state machine cannot defer events" );
public:
    state_machine()
        : state_machine::machine_type{},
          mutex_{} {}
    template<typename ... Args>
    explicit
    state_machine(Args&& ... args)
        : state_machine::machine_type(::std::forward<Args>(args)...),
          mutex_{}
    {}

    template < typename Event >
    actions::event_process_result
    process_event( Event&& evt )
    {
        lock_guard lock{mutex_};
        return process_event_impl(::std::forward<Event>(evt),
                detail::event_process_selector<
                    Event,
                    typename state_machine::handled_events>{} );
    }
private:
    template < typename Event >
    actions::event_process_result
    process_event_impl(Event&& evt,
        detail::process_type<actions::event_process_result::process> const&)
    {
        // TODO Dispatch event
        return actions::event_process_result::process;
    }
    template < typename Event >
    actions::event_process_result
    process_event_impl(Event&& evt,
        detail::process_type<actions::event_process_result::refuse> const&)
    {
        static_assert( detail::event_process_selector<
                Event,
                typename state_machine::handled_events,
                typename state_machine::deferred_events>::value
            != actions::event_process_result::refuse,
            "Event type is not handled by this state machine" );
        return actions::event_process_result::refuse;
    }
private:
    using mutex_type        = Mutex;
    using lock_guard        = typename detail::lock_guard_type<mutex_type>::type;

    mutex_type              mutex_;
};

}  /* namespace afsm */

#endif /* AFSM_FSM_HPP_ */
