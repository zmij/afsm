/*
 * fsm.hpp
 *
 *  Created on: 25 мая 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef AFSM_FSM_HPP_
#define AFSM_FSM_HPP_

#include <afsm/detail/base_states.hpp>
#include <deque>

namespace afsm {

template < typename FSM, typename T >
class state : public detail::state_base< T > {
public:
    using enclosing_fsm_type    = FSM;
public:
    state(enclosing_fsm_type& fsm)
        : state::state_type{}, fsm_{fsm}
    {}
    state(enclosing_fsm_type& fsm, state const& rhs)
        : state::state_type{rhs}, fsm_{fsm}
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

    enclosing_fsm_type&
    enclosing_fsm()
    { return fsm_; }
    enclosing_fsm_type const&
    enclosing_fsm() const
    { return fsm_; }

    template < typename Event >
    void
    state_enter(Event&&) {}
    template < typename Event >
    void
    state_exit(Event&&) {}
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
    process_event_impl(Event&&,
        detail::process_type<actions::event_process_result::defer> const&)
    {
        return actions::event_process_result::defer;
    }
    template < typename Event >
    actions::event_process_result
    process_event_impl(Event&&,
        detail::process_type<actions::event_process_result::refuse> const&)
    {
        return actions::event_process_result::refuse;
    }
private:
    enclosing_fsm_type&    fsm_;
};

template < typename FSM, typename T >
class inner_state_machine : public detail::state_machine_base< T, none, inner_state_machine<FSM, T> > {
public:
    using enclosing_fsm_type    = FSM;
    using this_type = inner_state_machine<FSM, T>;
    using base_machine_type     = detail::state_machine_base< T, none, this_type >;
public:
    inner_state_machine(enclosing_fsm_type& fsm)
        : base_machine_type{}, fsm_{fsm} {}
    inner_state_machine(enclosing_fsm_type& fsm, inner_state_machine const& rhs)
        : base_machine_type{rhs}, fsm_{fsm} {}

    template < typename Event >
    actions::event_process_result
    process_event( Event&& event )
    {
        return process_event_impl(fsm_, ::std::forward<Event>(event),
                detail::event_process_selector<
                    Event,
                    typename inner_state_machine::handled_events,
                    typename inner_state_machine::deferred_events>{} );
    }

    enclosing_fsm_type&
    enclosing_fsm()
    { return fsm_; }
    enclosing_fsm_type const&
    enclosing_fsm() const
    { return fsm_; }
private:
    using base_machine_type::process_event_impl;
private:
    enclosing_fsm_type&    fsm_;
};

template < typename T, typename Mutex >
class state_machine : public detail::state_machine_base< T, Mutex, state_machine<T, Mutex> > {
public:
    static_assert( ::psst::meta::is_empty< typename T::deferred_events >::value,
            "Outer state machine cannot defer events" );
    using this_type         = state_machine<T, Mutex>;
    using base_machine_type = detail::state_machine_base< T, Mutex, this_type >;
public:
    state_machine()
        : base_machine_type{},
          stack_size_{0},
          mutex_{},
          queued_events_{},
          deferred_mutex_{},
          deferred_events_{}
      {}
    template<typename ... Args>
    explicit
    state_machine(Args&& ... args)
        : base_machine_type(::std::forward<Args>(args)...),
          stack_size_{0},
          mutex_{},
          queued_events_{},
          deferred_mutex_{},
          deferred_events_{}
    {}

    template < typename Event >
    actions::event_process_result
    process_event( Event&& event )
    {
        if (!stack_size_++) {
            auto res = process_event_dispatch(::std::forward<Event>(event));
            lock_guard lock{mutex_};
            // Process enqueued events
            --stack_size_;
            return res;
        } else {
            --stack_size_;
            // Enqueue event
            enqueue_event(::std::forward<Event>(event));
            return actions::event_process_result::defer;
        }
    }
private:
    template < typename Event >
    actions::event_process_result
    process_event_dispatch( Event&& event )
    {
        return process_event_impl(::std::forward<Event>(event),
                    detail::event_process_selector<
                        Event,
                        typename state_machine::handled_events>{} );
    }

    template < typename Event >
    actions::event_process_result
    process_event_impl(Event&& event,
        detail::process_type<actions::event_process_result::process> const& sel)
    {
        using actions::event_process_result;
        auto res = base_machine_type::process_event_impl(*this, ::std::forward<Event>(event), sel );
        switch (res) {
            case event_process_result::process:
                // Changed state. Process deferred events
                process_deferred_queue();
                break;
            case event_process_result::defer:
                // Add event to deferred queue
                defer_event(::std::forward<Event>(event));
                break;
            case event_process_result::refuse:
                // The event cannot be processed in current state
                break;
            default:
                break;
        }
        return res;
    }
    template < typename Event >
    actions::event_process_result
    process_event_impl(Event&&,
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

    template < typename Event >
    void
    enqueue_event(Event&& event)
    {
        lock_guard lock{mutex_};
        Event evt = event;
        queued_events_.emplace_back([&, evt]() mutable {
            return process_event_dispatch(::std::move(evt));
        });
    }

    void
    process_event_queue()
    {

    }

    template < typename Event >
    void
    defer_event(Event&& event)
    {
        lock_guard lock{deferred_mutex_};
        Event evt = event;
        deferred_events_.emplace_back([&, evt]() mutable {
            return process_event_dispatch(::std::move(evt));
        });
    }
    void
    process_deferred_queue()
    {
        if (stack_size_++ <= 1) {

            event_queue deferred;
            {
                lock_guard lock{deferred_mutex_};
                ::std::swap(deferred_events_, deferred);
            }
            while (!deferred.empty()) {
                bool next_state = false;
                while (!next_state && !deferred.empty()) {
                    auto event = deferred.front();
                    deferred.pop_front();
                    next_state = event() == actions::event_process_result::process;
                }
                if (next_state) {
                    lock_guard lock{deferred_mutex_};
                    deferred_events_.insert(deferred_events_.end(), deferred.begin(), deferred.end());
                    ::std::swap(deferred_events_, deferred);
                }
            }
        }
        --stack_size_;
    }
private:
    using mutex_type        = Mutex;
    using lock_guard        = typename detail::lock_guard_type<mutex_type>::type;
    using atomic_counter    = ::std::atomic< ::std::size_t >;
    using event_invokation  = ::std::function< actions::event_process_result() >;
    using event_queue       = ::std::deque< event_invokation >;

    atomic_counter          stack_size_;

    mutex_type              mutex_;
    event_queue             queued_events_;

    mutex_type              deferred_mutex_;
    event_queue             deferred_events_;
};

}  /* namespace afsm */

#endif /* AFSM_FSM_HPP_ */
