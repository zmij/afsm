/*
 * meta.hpp
 *
 *  Created on: 26 мая 2016 г.
 *      Author: sergey.fedorov
 */

#include <type_traits>

#ifndef META_HPP_
#define META_HPP_

namespace afsm {
namespace meta {

template < size_t N, typename ... T >
struct nth_type;

template < size_t N, typename T, typename ... Y >
struct nth_type< N, T, Y ... > : nth_type < N - 1, Y ... > {
    static_assert(N <= sizeof ...(Y), "Index type is out of range");
};

template < typename T, typename ... Y >
struct nth_type < 0, T, Y ... > {
    using type = T;
};

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

template < typename ... T >
struct type_tuple {
    static constexpr size_t size = sizeof ... (T);
    template < size_t N >
    using type = typename nth_type<N, T ...>::type;
};

template < typename T, typename Y >
struct combine;

template < typename ... T, typename ... Y >
struct combine< type_tuple<T...>, type_tuple<Y...> > {
    using type = type_tuple<T..., Y...>;
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

template < typename ... T, typename ... Y >
struct type_set< type_tuple<T...>, type_tuple<Y...> >
    : type_set<T..., Y...> {};

template < typename ... T, typename ... Y >
struct type_set< type_set<T...>, type_set<Y...> >
    : type_set<T..., Y...> {};

template < typename T, typename ... Y >
struct contains< type_set<Y...>, T > : contains<T, Y...> {};

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

    static constexpr size_t size = key_types::size;

    template < size_t N >
    using type = type_pair<
            typename key_types::template type<N>,
            typename value_types::template type<N>>;
};

template <>
struct type_map<> {
    using key_types     = type_tuple<>;
    using value_type    = type_tuple<>;
};

}  /* namespace meta */
}  /* namespace afsm */

#endif /* META_HPP_ */
