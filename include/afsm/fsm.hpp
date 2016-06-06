/*
 * fsm.hpp
 *
 *  Created on: 25 мая 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef AFSM_FSM_HPP_
#define AFSM_FSM_HPP_

#include <afsm/detail/base_states.hpp>
#include <afsm/detail/observer.hpp>
#include <deque>

namespace afsm {

//----------------------------------------------------------------------------
//  State
//----------------------------------------------------------------------------
template < typename T, typename FSM >
class state : public detail::state_base< T > {
public:
    using enclosing_fsm_type    = FSM;
public:
    state(enclosing_fsm_type& fsm)
        : state::state_type{}, fsm_{&fsm}
    {}
    state(state const& rhs) = default;
    state(state&& rhs) = default;
    state(enclosing_fsm_type& fsm, state const& rhs)
        : state::state_type{static_cast<typename state::state_type const&>(rhs)}, fsm_{&fsm}
    {}
    state(enclosing_fsm_type& fsm, state&& rhs)
        : state::state_type{static_cast<typename state::state_type&&>(rhs)}, fsm_{&fsm}
    {}

    state&
    operator = (state const& rhs)
    {
        state{rhs}.swap(*this);
        return *this;
    }
    state&
    operator = (state&& rhs)
    {
        swap(rhs);
        return *this;
    }

    void
    swap(state& rhs) noexcept
    {
        static_cast<typename state::state_type&>(*this).swap(rhs);
    }

    template < typename Event >
    actions::event_process_result
    process_event( Event&& evt )
    {
        return process_event_impl(::std::forward<Event>(evt),
                detail::event_process_selector<
                    Event,
                    typename state::internal_events,
                    typename state::deferred_events>{} );
    }

    enclosing_fsm_type&
    enclosing_fsm()
    { return *fsm_; }
    enclosing_fsm_type const&
    enclosing_fsm() const
    { return *fsm_; }
    void
    enclosing_fsm(enclosing_fsm_type& fsm)
    { fsm_ = &fsm; }

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
        return actions::handle_in_state_event(::std::forward<Event>(evt), *fsm_, *this);
    }
    template < typename Event >
    constexpr actions::event_process_result
    process_event_impl(Event&&,
        detail::process_type<actions::event_process_result::defer> const&)
    {
        return actions::event_process_result::defer;
    }
    template < typename Event >
    constexpr actions::event_process_result
    process_event_impl(Event&&,
        detail::process_type<actions::event_process_result::refuse> const&)
    {
        return actions::event_process_result::refuse;
    }
private:
    enclosing_fsm_type*    fsm_;
};

//----------------------------------------------------------------------------
//  Inner state machine
//----------------------------------------------------------------------------
template < typename T, typename FSM >
class inner_state_machine : public detail::state_machine_base< T, none, inner_state_machine<T, FSM> > {
public:
    using enclosing_fsm_type    = FSM;
    using this_type = inner_state_machine<T, FSM>;
    using base_machine_type     = detail::state_machine_base< T, none, this_type >;
public:
    inner_state_machine(enclosing_fsm_type& fsm)
        : base_machine_type{this}, fsm_{&fsm} {}
    inner_state_machine(inner_state_machine const& rhs)
        : base_machine_type{this, rhs}, fsm_{rhs.fsm_} {}
    inner_state_machine(inner_state_machine&& rhs)
        : base_machine_type{this, ::std::move(rhs)},
          fsm_{rhs.fsm_}
    {
    }

    inner_state_machine(enclosing_fsm_type& fsm, inner_state_machine const& rhs)
        : base_machine_type{static_cast<base_machine_type const&>(rhs)}, fsm_{&fsm} {}
    inner_state_machine(enclosing_fsm_type& fsm, inner_state_machine&& rhs)
        : base_machine_type{static_cast<base_machine_type&&>(rhs)}, fsm_{&fsm} {}

    void
    swap(inner_state_machine& rhs) noexcept
    {
        using ::std::swap;
        static_cast<base_machine_type&>(*this).swap(rhs);
        swap(fsm_, rhs.fsm_);
    }
    inner_state_machine&
    operator = (inner_state_machine const& rhs)
    {
        inner_state_machine tmp{rhs};
        swap(tmp);
        return *this;
    }
    inner_state_machine&
    operator = (inner_state_machine&& rhs)
    {
        swap(rhs);
        return *this;
    }

    template < typename Event >
    actions::event_process_result
    process_event( Event&& event )
    {
        return process_event_impl(*fsm_, ::std::forward<Event>(event),
                detail::event_process_selector<
                    Event,
                    typename inner_state_machine::handled_events,
                    typename inner_state_machine::deferred_events>{} );
    }

    enclosing_fsm_type&
    enclosing_fsm()
    { return *fsm_; }
    enclosing_fsm_type const&
    enclosing_fsm() const
    { return *fsm_; }
    void
    enclosing_fsm(enclosing_fsm_type& fsm)
    { fsm_ = &fsm; }
private:
    using base_machine_type::process_event_impl;
private:
    enclosing_fsm_type*    fsm_;
};

//----------------------------------------------------------------------------
//  State machine
//----------------------------------------------------------------------------
template < typename T, typename Mutex, typename Observer,
        template<typename> class ObserverWrapper >
class state_machine :
        public detail::state_machine_base< T, Mutex,
            state_machine<T, Mutex, Observer> >,
        public ObserverWrapper<Observer> {
public:
    static_assert( ::psst::meta::is_empty< typename T::deferred_events >::value,
            "Outer state machine cannot defer events" );
    using this_type         = state_machine<T, Mutex, Observer>;
    using base_machine_type = detail::state_machine_base< T, Mutex, this_type >;
    using mutex_type        = Mutex;
    using lock_guard        = typename detail::lock_guard_type<mutex_type>::type;
    using observer_wrapper  = ObserverWrapper<Observer>;
public:
    state_machine()
        : base_machine_type{this},
          stack_size_{0},
          mutex_{},
          queued_events_{},
          deferred_mutex_{},
          deferred_events_{}
      {}
    template<typename ... Args>
    explicit
    state_machine(Args&& ... args)
        : base_machine_type(this, ::std::forward<Args>(args)...),
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
            // Process enqueued events
            process_event_queue();
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
        observer_wrapper::start_process_event(*this, ::std::forward<Event>(event));
        auto res = base_machine_type::process_event_impl(*this, ::std::forward<Event>(event), sel );
        switch (res) {
            case event_process_result::process:
                observer_wrapper::state_changed(*this);
                // Changed state. Process deferred events
                process_deferred_queue();
                break;
            case event_process_result::process_in_state:
                observer_wrapper::processed_in_state(*this, ::std::forward<Event>(event));
                break;
            case event_process_result::defer:
                // Add event to deferred queue
                defer_event(::std::forward<Event>(event));
                break;
            case event_process_result::refuse:
                // The event cannot be processed in current state
                observer_wrapper::reject_event(*this, ::std::forward<Event>(event));
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
        observer_wrapper::enqueue_event(*this, ::std::forward<Event>(event));
        Event evt = event;
        queued_events_.emplace_back([&, evt]() mutable {
            return process_event_dispatch(::std::move(evt));
        });
    }

    void
    process_event_queue()
    {
        event_queue postponed;
        {
            lock_guard lock{mutex_};
            ::std::swap(queued_events_, postponed);
        }
        while (!postponed.empty()) {
            observer_wrapper::start_process_events_queue(*this);
            for (auto const& event : postponed) {
                event();
            }
            {
                lock_guard lock{mutex_};
                postponed.clear();
                ::std::swap(queued_events_, postponed);
            }
            observer_wrapper::end_process_events_queue(*this);
        }
    }

    template < typename Event >
    void
    defer_event(Event&& event)
    {
        lock_guard lock{deferred_mutex_};
        observer_wrapper::defer_event(*this, ::std::forward<Event>(event));
        Event evt = event;
        deferred_events_.emplace_back([&, evt]() mutable {
            return process_event_dispatch(::std::move(evt));
        });
    }
    void
    process_deferred_queue()
    {
        using actions::event_process_result;
        if (stack_size_++ <= 1) {
            event_queue deferred;
            {
                lock_guard lock{deferred_mutex_};
                ::std::swap(deferred_events_, deferred);
            }
            while (!deferred.empty()) {
                observer_wrapper::start_process_deferred_queue(*this);
                auto res = event_process_result::refuse;
                while (!deferred.empty()) {
                    auto event = deferred.front();
                    deferred.pop_front();
                    res = event();
                    if (res == event_process_result::process)
                        break;
                }
                {
                    lock_guard lock{deferred_mutex_};
                    deferred_events_.insert(deferred_events_.end(), deferred.begin(), deferred.end());
                    deferred.clear();
                }
                if (res == event_process_result::process) {
                    ::std::swap(deferred_events_, deferred);
                }
                observer_wrapper::end_process_deferred_queue(*this);
            }
        }
        --stack_size_;
    }
private:
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
