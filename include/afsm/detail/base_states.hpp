/*
 * base_states.hpp
 *
 *  Created on: 28 мая 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef AFSM_DETAIL_BASE_STATES_HPP_
#define AFSM_DETAIL_BASE_STATES_HPP_

#include <afsm/definition.hpp>
#include <mutex>
#include <atomic>

namespace afsm {
namespace detail {

template <typename ... T>
struct not_a_state;

template < typename T >
struct not_a_state<T> {
    static_assert( def::detail::is_state<T>::value, "Type is not a state" );
};

template < typename FSM, typename T >
struct front_state_type
    : ::std::conditional<
        def::detail::is_state_machine< T >::value,
        inner_state_machine< FSM, T >,
        typename ::std::conditional<
            def::detail::is_state< T >::value,
            state< FSM, T >,
            not_a_state< T >
        >::type
    > {};

template < typename FSM, typename T >
struct front_state_tuple;

template < typename FSM >
struct front_state_tuple< FSM, void> {
    using type = ::std::tuple<>;

    static type
    construct(FSM&)
    { return type{}; }
};

template < typename FSM, typename ... T>
struct front_state_tuple< FSM, meta::type_tuple<T...> > {
    using type = ::std::tuple<
            typename front_state_type<FSM, T>::type ... >;

    static type
    construct(FSM& fsm)
    { return type( typename front_state_type<FSM, T>::type{fsm}... ); }
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

enum class event_process_result {
    process,
    defer,
    refuse
};

template < event_process_result R >
using process_type = ::std::integral_constant< event_process_result, R >;

template <typename Event, typename HandledEvents,
        typename DeferredEvents = meta::type_tuple<>>
struct event_process_selector
    : ::std::conditional<
        meta::contains<Event, HandledEvents>::value,
        process_type<event_process_result::process>,
        typename ::std::conditional<
            meta::contains<Event, DeferredEvents>::value,
            process_type<event_process_result::defer>,
            process_type<event_process_result::refuse>
        >::type
    >::type{};

template < typename T >
class state_base : public T {
public:
    using state_definition_type = T;
    using state_type            = state_base<T>;
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
    using deferred_events =
            typename ::std::conditional<
                ::std::is_same<typename T::deferred_events, void>::value,
                meta::type_tuple<>,
                typename meta::type_set< typename T::deferred_events >::type
            >::type;
public:
    state_base() : state_definition_type{} {}
protected:
    template< typename ... Args >
    state_base(Args&& ... args)
        : state_definition_type(::std::forward<Args>(args)...) {}
};

template < typename T, typename Mutex >
class state_machine_base : public state_base<T> {
public:
    using state_machine_definition_type = T;
    using state_type                    = state_base<T>;
    using machine_type                  = state_machine_base<T, Mutex>;
    static_assert(def::detail::is_state_machine<T>::value,
            "Front state machine can be created only with a descendant of afsm::def::state_machine");
    using transitions = typename state_machine_definition_type::transitions;
    using handled_events =
            typename def::detail::recursive_handled_events<state_machine_definition_type>::type;
    static_assert(def::detail::is_state<
                typename state_machine_definition_type::initial_state >::value,
            "State machine definition must specify an initial state");
    using initial_state = typename state_machine_definition_type::initial_state;
    using inner_states_def =
            typename def::detail::inner_states< transitions >::type;
    using inner_states_constructor =
            front_state_tuple< machine_type, inner_states_def >;
    using inner_states_tuple =
            typename inner_states_constructor::type;

    static constexpr ::std::size_t initial_state_index =
            meta::index_of<initial_state, inner_states_def>::value;
    static constexpr ::std::size_t inner_state_count = inner_states_def::size;

    using type_indexes = typename meta::index_builder<inner_state_count>::type;

    state_machine_base()
        : state_type{},
          current_state_{initial_state_index},
          inner_states_{ inner_states_constructor::construct(*this) }
    {}

    state_machine_base(state_machine_base const&) = delete;
    state_machine_base&
    operator = (state_machine_base const&) = delete;

    state_machine_base(state_machine_base&&) = default;
    state_machine_base&
    operator = (state_machine_base&&) = default;

    template < ::std::size_t N>
    ::std::tuple_element< N, inner_states_tuple >&
    get_state()
    { return ::std::get<N>(inner_states_); }
    template < ::std::size_t N>
    ::std::tuple_element< N, inner_states_tuple > const&
    get_state() const
    { return ::std::get<N>(inner_states_); }

    ::std::size_t
    current_state() const
    { return (::std::size_t)current_state_; }
protected:
    template<typename ... Args>
    explicit
    state_machine_base(Args&& ... args)
        : state_type(::std::forward<Args>(args)...),
          current_state_{initial_state_index}
    {}
protected:
    using mutex_type        = Mutex;
    using size_type         = typename detail::size_type<mutex_type>::type;

    size_type               current_state_;
    inner_states_tuple      inner_states_;
};

template < typename T, typename Mutex >
constexpr ::std::size_t state_machine_base<T, Mutex>::initial_state_index;
template < typename T, typename Mutex >
constexpr ::std::size_t state_machine_base<T, Mutex>::inner_state_count;

}  /* namespace detail */
}  /* namespace afsm */



#endif /* AFSM_DETAIL_BASE_STATES_HPP_ */
