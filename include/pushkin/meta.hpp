/*
 * meta.hpp
 *
 *  Created on: 26 мая 2016 г.
 *      Author: sergey.fedorov
 */

#include <type_traits>
#include <limits>

#include <pushkin/meta/algorithm.hpp>
#include <pushkin/meta/index_tuple.hpp>
#include <pushkin/meta/callable.hpp>

#ifndef PUSHKIN_META_HPP_
#define PUSHKIN_META_HPP_

namespace psst {
namespace meta {

template < typename ... T >
struct type_set;

template < typename T, typename ... Y >
struct type_set<T, Y...> {
    using type = typename insert_type<
            typename type_set<Y...>::type, T>::type;
};

template <>
struct type_set<> {
    using type = type_tuple<>;
};

template < typename ... T >
struct type_set< type_tuple<T...> > : type_set<T...> {};
template < typename ... T, typename ... Y >
struct type_set< type_tuple<T...>, type_tuple<Y...> >
    : type_set<T..., Y...> {};

template < typename ... T, typename ... Y >
struct type_set< type_set<T...>, type_set<Y...> >
    : type_set<T..., Y...> {};

template < typename T, typename ... Y >
struct contains< T, type_set<Y...> > : contains<T, Y...> {};

template < typename ... T >
struct type_map_base;

template < typename ... K, typename ... V >
struct type_map_base< type_tuple<K...>, type_tuple<V...> > {
    using key_types     = type_tuple<K...>;
    using value_types   = type_tuple<V...>;
};

template <>
struct type_map_base< type_tuple<>, type_tuple<> > {
    using key_types     = type_tuple<>;
    using value_type    = type_tuple<>;
};

template < typename ... T >
struct key_exists;
template < typename T, typename Y >
struct key_exists<T, Y> {
    static_assert(!::std::is_same<T, Y>::value, "Key type already exists");
};


template <typename ... T>
struct insert_type_pair;
template < typename K, typename V, typename ... T, typename ... U >
struct insert_type_pair< type_map_base< type_tuple<T...>, type_tuple<U...>>, K, V >
    : ::std::conditional< contains< K, T ... >::value,
        key_exists< K, K >,
        type_map_base< type_tuple<K, T...>, type_tuple<V, U...> >
        >::type {};

template <typename K, typename V>
struct type_pair {
    using key_type      = K;
    using value_type    = V;
};

template <typename ... T>
struct type_map;

template < typename K, typename V, typename ... T, typename ... Y >
struct type_map< type_tuple< K, T... >, type_tuple<V, Y...> > {
    using unique_keys =
            insert_type_pair<
                type_map_base< type_tuple<T...>, type_tuple<Y...> >, K, V >;
    using key_types     = typename unique_keys::key_types;
    using value_types   = typename unique_keys::value_types;

    static_assert(key_types::size == value_types::size,
            "Incorrect size of type_map");

    static constexpr ::std::size_t size = key_types::size;

    template < ::std::size_t N >
    using type = type_pair<
            typename key_types::template type<N>,
            typename value_types::template type<N>>;
};

template <>
struct type_map<> {
    using key_types     = type_tuple<>;
    using value_type    = type_tuple<>;

    static constexpr ::std::size_t size = 0;
};

template <>
struct type_map< type_tuple<>, type_tuple<> > : type_map<> {};

}  /* namespace meta */
}  /* namespace pus */

#endif /* PUSHKIN_META_HPP_ */
