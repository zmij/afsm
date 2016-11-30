/*
 * algorithm.hpp
 *
 *  Created on: 28 мая 2016 г.
 *      Author: sergey.fedorov
 */

/**
 * @page Metaprogramming algorithms
 *
 */
#ifndef PUSHKIN_META_ALGORITHM_HPP_
#define PUSHKIN_META_ALGORITHM_HPP_

#include <type_traits>
#include <limits>

#include <pushkin/meta/type_tuple.hpp>

namespace psst {
namespace meta {

/**
 * Metafunction to determine if typename T is contained in type
 * variadic pack Y ...
 */
template < typename T, typename ... Y >
struct contains;
template < typename T, typename V, typename ... Y >
struct contains< T, V, Y ... > : contains < T, Y ...> {};
template < typename T, typename ... Y >
struct contains< T, T, Y ... > : ::std::true_type {};
template < typename T >
struct contains< T, T > : ::std::true_type {};
template < typename T, typename Y >
struct contains< T, Y > : ::std::false_type {};
template < typename T >
struct contains<T> : ::std::false_type {};
template < typename T, typename ... Y >
struct contains< T, type_tuple<Y...> > : contains<T, Y...> {};

/**
 * Metafunction to determine if a variadic pack is empty
 */
template < typename ... T >
struct is_empty : ::std::false_type {};
template <>
struct is_empty<> : ::std::true_type {};
template <>
struct is_empty<void> : ::std::true_type {};
template < typename ... T >
struct is_empty< type_tuple<T...> >
    : ::std::conditional<
        (sizeof ... (T) > 0),
        ::std::false_type,
        ::std::true_type
    >::type {};

template < typename ... T >
struct front;

template < typename T, typename ... Y >
struct front<T, Y...> {
    using type = T;
};

template < typename ... T >
struct front< type_tuple<T...> > : front<T...> {};

namespace detail {

template < typename T, ::std::size_t N, typename ... Y >
struct index_of_impl;

template < typename T, ::std::size_t N, typename V, typename ... Y >
struct index_of_impl< T, N, V, Y ... > : index_of_impl<T, N + 1, Y...> {};
template < typename T, ::std::size_t N, typename ... Y >
struct index_of_impl< T, N, T, Y ... > {
    static constexpr ::std::size_t value    = N;
    static constexpr bool found             = true;
};

template < typename T, ::std::size_t N >
struct index_of_impl< T, N, T > {
    static constexpr ::std::size_t value    = N;
    static constexpr bool found             = true;
};

template < typename T, ::std::size_t N, typename Y>
struct index_of_impl<T, N, Y> {
    static constexpr ::std::size_t value    = ::std::numeric_limits<::std::size_t>::max();
    static constexpr bool found             = false;
};

template < typename T, ::std::size_t N>
struct index_of_impl<T, N> {
    static constexpr ::std::size_t value    = ::std::numeric_limits<::std::size_t>::max();
    static constexpr bool found             = false;
};

}  /* namespace detail */

template < typename T, typename ... Y >
struct index_of : detail::index_of_impl<T, 0, Y...> {};
template < typename T, typename ... Y >
struct index_of< T, type_tuple<Y...> > : detail::index_of_impl<T, 0, Y...> {};

template < typename ... T >
struct combine;

template <>
struct combine<> {
    using type = type_tuple<>;
};

template < typename T >
struct combine < T > {
    using type = type_tuple<T>;
};

template < typename ... T >
struct combine < type_tuple<T...> > {
    using type = type_tuple<T...>;
};

template < typename ... T, typename U, typename ... Y >
struct combine< type_tuple<T...>, U, Y...>
    : combine< type_tuple<T..., U>, Y...> {};

template < typename T, typename ... U, typename ... Y >
struct combine< T, type_tuple<U...>, Y... > :
    combine<type_tuple<T, U...>, Y...>{};


template < typename ... T, typename ... U, typename ... Y >
struct combine< type_tuple<T...>, type_tuple<U...>, Y... > {
    using type = typename combine< type_tuple<T..., U... >, Y...>::type;
};

template < typename T, typename ... Y >
struct push_back {
    using type = type_tuple<Y..., T>;
};
template < typename T, typename ... Y >
struct push_back<type_tuple<Y...>, T > {
    using type = type_tuple<Y..., T>;
};

template < typename T, typename ... Y >
struct push_front {
    using type = type_tuple<T, Y...>;
};
template < typename T, typename ... Y >
struct push_front<type_tuple<Y...>, T > {
    using type = type_tuple<T, Y...>;
};

template < typename ... T >
struct pop_front;

template < typename T, typename ... Y >
struct pop_front<T, Y...> {
    using type = type_tuple<Y...>;
};

template <>
struct pop_front<> {
    using type = type_tuple<>;
};

template < typename ... T >
struct pop_front< type_tuple<T...> > : pop_front<T...> {};

template < typename T, typename ... Y>
struct remove_type;

template < typename T, typename V, typename ... Y >
struct remove_type<T, V, Y ...> {
    using type = typename push_front<
            typename remove_type<T, Y...>::type, V >::type;
};

template < typename T, typename ... Y >
struct remove_type<T, T, Y...> {
    using type = typename remove_type<T, Y...>::type;
};

template < typename T >
struct remove_type <T> {
    using type = type_tuple<>;
};

template < typename T, typename ... Y >
struct remove_type< type_tuple<Y...>, T > {
    using type = typename remove_type<T, Y...>::type;
};

template < typename T, typename ... Y >
struct noop {
    using type = type_tuple<Y...>;
};

template < typename T, typename ... Y >
struct insert_type : ::std::conditional<
        contains<T, Y...>::value,
        noop<T, Y...>,
        typename ::std::conditional<
            ::std::is_same< T, void >::value,
            noop<T, Y...>,
            push_front<T, Y...>
        >::type
    >::type {};

template < typename T, typename ... Y >
struct insert_type< type_tuple<Y...>, T > {
    using type = typename insert_type<T, Y...>::type;
};

template < typename ... T >
struct unique;

template < typename T, typename ... Y >
struct unique<T, Y...> {
    using type = typename insert_type<
            typename unique<Y...>::type, T>::type;
};

template <>
struct unique<> {
    using type = type_tuple<>;
};

template < typename ... T >
struct unique< type_tuple<T...> > : unique<T...> {};
template < typename ... T, typename ... Y >
struct unique< type_tuple<T...>, type_tuple<Y...> >
    : unique<T..., Y...> {};

template < typename ... T, typename ... Y >
struct unique< unique<T...>, unique<Y...> >
    : unique<T..., Y...> {};

template < typename T, typename ... Y >
struct contains< T, unique<Y...> > : contains<T, Y...> {};

template < template <typename> class Predicate, typename ... T >
struct all_match;

template < template <typename> class Predicate, typename T, typename ... Y >
struct all_match< Predicate, T, Y... >
    : ::std::conditional<
        Predicate<T>::value,
        all_match<Predicate, Y...>,
        ::std::false_type
    >::type {};

template < template <typename> class Predicate, typename T >
struct all_match< Predicate, T >
    : ::std::conditional<
        Predicate<T>::value,
        ::std::true_type,
        ::std::false_type
    >::type {};

template < template <typename> class Predicate >
struct all_match< Predicate >
    : ::std::false_type {};

template < template <typename> class Predicate, typename ... T >
struct all_match< Predicate, type_tuple<T...> >
    : all_match<Predicate, T...> {};

template < template <typename> class Predicate, typename ... T>
struct any_match;

template < template <typename> class Predicate, typename T, typename ... Y >
struct any_match< Predicate, T, Y... >
    : ::std::conditional<
        Predicate<T>::value,
        ::std::true_type,
        any_match<Predicate, Y ...>
    >::type {};

template < template <typename> class Predicate, typename T >
struct any_match< Predicate, T >
    : std::conditional<
        Predicate<T>::value,
        ::std::true_type,
        ::std::false_type
    >::type {};

template < template <typename> class Predicate >
struct any_match< Predicate > : ::std::false_type {};

template < template <typename> class Predicate, typename ... T >
struct any_match< Predicate, type_tuple<T...> >
    : any_match< Predicate, T... >{};

namespace detail {

}  /* namespace detail */

template < template <typename> class Predicate, typename ... T >
struct find_if;

template <template <typename> class Predicate>
struct find_if<Predicate> {
    using type = type_tuple<>;
};

template < template <typename> class Predicate, typename T, typename ... Y>
struct find_if< Predicate, T, Y... >
    : ::std::conditional<
        Predicate<T>::value,
        combine< T, typename find_if<Predicate, Y...>::type>,
        find_if<Predicate, Y...>
    >::type {};

template < template <typename> class Predicate, typename T >
struct find_if< Predicate, T >
    : ::std::conditional<
        Predicate<T>::value,
        combine<T>,
        combine<>
    >::type {};

template < template <typename> class Predicate, typename ... T>
struct find_if< Predicate, type_tuple<T...> > : find_if< Predicate, T... > {};
template < template<typename> class Predicate, typename ... T >
struct transform {
    using type = type_tuple< typename Predicate<T>::type ... >;
};

template < template<typename> class Predicate, typename ... T >
struct transform < Predicate, type_tuple<T...> >
    : transform< Predicate, T... > {};

template < template<typename> class Predicate >
struct transform<Predicate> {
    using type = type_tuple<>;
};

template < template<typename> class Predicate, typename T >
struct invert {
    static constexpr bool value = !Predicate<T>::value;
};

}  /* namespace meta */
}  /* namespace pus */

#endif /* PUSHKIN_META_ALGORITHM_HPP_ */
