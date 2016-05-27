/*
 * fsm.hpp
 *
 *  Created on: 25 мая 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef AFSM_FSM_HPP_
#define AFSM_FSM_HPP_

#include <afsm/definition.hpp>
#include <mutex>
#include <atomic>

namespace afsm {

template < typename T >
struct state;
template < typename T, typename Mutex = none >
struct state_machine;

namespace detail {

template <typename ... T>
struct not_a_state;

template < typename T >
struct not_a_state<T> {
    static_assert( def::detail::is_state<T>::value, "Type is not a state" );
};

template < typename T, typename Mutex >
struct front_state_type
    : ::std::conditional<
        def::detail::is_state_machine< T >::value,
        state_machine< T, Mutex >,
        typename ::std::conditional<
            def::detail::is_state< T >::value,
            state< T >,
            not_a_state< T >
        >::type
    > {};

template < typename Mutex, typename T >
struct front_state_tuple;

template < typename Mutex >
struct front_state_tuple<Mutex, void> {
    using type = ::std::tuple<>;
};

template <typename Mutex, typename ... T>
struct front_state_tuple< Mutex, meta::type_tuple<T...> > {
    using type = ::std::tuple<
            typename front_state_type<T, Mutex>::type ... >;
};

struct no_lock {
    no_lock(none&);
};

template < typename Mutex >
struct lock_guard_type {
    using type = ::std::lock_guard<Mutex>;
};

template <>
struct lock_guard_type<none> {
    using type = no_lock;
};
template <>
struct lock_guard_type<none&> {
    using type = no_lock;
};

template <typename Mutex>
struct size_type {
    using type = ::std::atomic<::std::size_t>;
};

template <>
struct size_type<none> {
    using type = ::std::size_t;
};
template <>
struct size_type<none&> {
    using type = ::std::size_t;
};

}  /* namespace detail */

template < typename T >
class state : T {
public:
    using state_definition_type = T;
    using internal_transitions = typename state_definition_type::internal_transitions;

    static_assert(def::detail::is_state<T>::value,
            "Front state can be created only with a descendant of afsm::def::state");
    static_assert(!def::detail::has_inner_states< internal_transitions >::value,
            "Internal transition table cannot have transitions between other states");
    using handled_events =
            typename def::detail::handled_events<state_definition_type>::type;
    static_assert(def::detail::is_state_machine<state_definition_type>::value
                || !def::detail::has_default_transitions< handled_events >::value,
            "Internal transition cannot be a default transition");
public:
    state() : state_definition_type{} {}
    template< typename ... Args >
    state(Args&& ... args)
        : state_definition_type(::std::forward<Args>(args)...) {}
};

template < typename T, typename Mutex >
class state_machine : state<T> {
public:
    using state_machine_definition_type  = T;
    using state_type                     = state<T>;
    static_assert(def::detail::is_state_machine<T>::value,
            "Front state machine can be created only with a descendant of afsm::def::state_machine");
    using transitions = typename state_machine_definition_type::transitions;
    using handled_events =
            typename def::detail::recursive_handled_events<state_machine_definition_type>::type;
    static_assert(def::detail::is_state<
                typename state_machine_definition_type::initial_state >::value,
            "State machine definition must specify an initial state");
    using initial_state = typename state_machine_definition_type::initial_state;
    using inner_states =
            typename def::detail::inner_states< transitions >::type;

    state_machine()
        : mutex_{}, state_type{}
    {}

    template<typename ... Args>
    state_machine(Args&& ... args)
        : mutex_{}, state_type(::std::forward<Args>(args)...)
    {}

    ::std::size_t
    current_state() const
    { return (::std::size_t)current_state_; }

    template < typename Event >
    void
    process_event(Event&& evt)
    {

    }
private:
protected:
    using mutex_type    = Mutex;
    using lock_guard    = typename detail::lock_guard_type<mutex_type>::type;
    using size_type     = typename detail::size_type<mutex_type>::type;
    using states_tuple  = typename detail::front_state_tuple< none, inner_states >::type;

    mutex_type          mutex_;
    size_type           current_state_;
    states_tuple        inner_states_;
};


}  /* namespace afsm */

#endif /* AFSM_FSM_HPP_ */
