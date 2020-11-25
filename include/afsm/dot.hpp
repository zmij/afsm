/**
 * Copyright 2019 Sergei A. Fedorov
 * dot.hpp
 *
 *  Created on: Mar 16, 2019
 *      Author: ser-fedorov
 */

#ifndef AFSM_DOT_HPP_
#define AFSM_DOT_HPP_

#include <iostream>
#include <type_traits>
#include <utility>

namespace afsm::dot {

template <char... C>
struct string {};

template <typename T, T... C>
constexpr string<C...> operator""_string()
{
    return {};
}

template <char... C>
constexpr bool
operator==(string<C...> const&, string<C...> const&)
{
    return true;
}

template <char... A, char... B>
constexpr bool
operator==(string<A...> const&, string<B...> const&)
{
    return false;
}

namespace detail {
template <typename T, typename Y>
struct begins_with_impl : std::false_type {};
template <char... A, char... B>
struct begins_with_impl<string<A...>, string<A..., B...>> : std::true_type {};
}    // namespace detail

template <char... A, char... B>
constexpr bool
begins_with(string<A...> const&, string<B...> const&)
{
    return detail::begins_with_impl<string<A...>, string<B...>>{};
}

template <char... A, char... B>
string<A..., B...>
operator+(string<A...>, string<B...>)
{
    return {};
}

template <char... C>
std::ostream&
operator<<(std::ostream& os, string<C...> const&)
{
    return (os << ... << C);
}

}    // namespace afsm::dot

#endif /* AFSM_DOT_HPP_ */
