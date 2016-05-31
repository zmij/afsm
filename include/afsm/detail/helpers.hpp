/*
 * helpers.hpp
 *
 *  Created on: 29 мая 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef AFSM_DETAIL_HELPERS_HPP_
#define AFSM_DETAIL_HELPERS_HPP_

#include <type_traits>
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
struct front_state_tuple< FSM, ::psst::meta::type_tuple<T...> > {
    using type = ::std::tuple<
            typename front_state_type<FSM, T>::type ... >;
    using index_tuple = typename ::psst::meta::index_builder< sizeof ... (T) >::type;

    static type
    construct(FSM& fsm)
    { return type( typename front_state_type<FSM, T>::type{fsm}... ); }
    static type
    copy_construct(FSM& fsm, type const& rhs)
    {
        return copy_construct(fsm, rhs, index_tuple{});
    }
    static type
    move_construct(FSM& fsm, type&& rhs)
    {
        return move_construct(fsm, ::std::forward<type>(rhs), index_tuple{});
    }
private:
    template < ::std::size_t ... Indexes >
    static type
    copy_construct(FSM& fsm, type const& rhs, ::psst::meta::indexes_tuple<Indexes...> const&)
    {
        return type( typename front_state_type<FSM, T>::type{
            fsm, ::std::get< Indexes >(rhs)}...);
    }
    template < ::std::size_t ... Indexes >
    static type
    move_construct(FSM& fsm, type&& rhs, ::psst::meta::indexes_tuple<Indexes...> const&)
    {
        return type( typename front_state_type<FSM, T>::type{
            fsm, ::std::move(::std::get< Indexes >(rhs))}...);
    }
};

struct no_lock {
    no_lock(none&) {}
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
}  /* namespace afsm */

#endif /* AFSM_DETAIL_HELPERS_HPP_ */
