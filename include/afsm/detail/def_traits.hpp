/*
 * def_traits.hpp
 *
 *  Created on: 1 июня 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef AFSM_DETAIL_DEF_TRAITS_HPP_
#define AFSM_DETAIL_DEF_TRAITS_HPP_

#include <afsm/definition_fwd.hpp>
#include <afsm/detail/tags.hpp>

namespace afsm {
namespace def {
namespace traits {

template < typename T >
struct is_transition : ::std::false_type {};

template < typename SourceState, typename Event, typename TargetState,
    typename Action, typename Guard>
struct is_transition< transition<SourceState, Event, TargetState, Action, Guard> >
    : ::std::true_type{};
template < typename Event, typename Action, typename Guard >
struct is_transition< internal_transition< Event, Action, Guard > >
    : ::std::true_type {};

template < typename T >
struct is_internal_transition : ::std::false_type {};

template < typename Event, typename Action, typename Guard >
struct is_internal_transition< internal_transition< Event, Action, Guard > >
    : ::std::true_type {};

template < typename T >
struct is_state
    : ::std::is_base_of< tags::state, T > {};

template < typename T >
struct is_terminal_state
    : ::std::is_base_of< terminal_state<T>, T > {};

template < typename T >
struct is_state_machine
    : ::std::is_base_of< tags::state_machine, T > {};

template < typename T >
struct has_common_base
    : ::std::is_base_of< tags::has_common_base, T > {};

template < typename T >
struct has_history
    : ::std::is_base_of< tags::has_history, T > {};

template < typename T >
struct allow_empty_transition_functions
    : ::std::is_base_of< tags::allow_empty_enter_exit, T > {};

namespace detail {
template < typename T, bool HasCommonBase >
struct inner_states_def {
    using definition_type   = T;
    using common_base_tag   = typename definition_type::common_base_tag_type;

    template < typename U, typename ... Tags >
    using state             = def::state<U, common_base_tag, Tags...>;
    template < typename U, typename ... Tags >
    using terminal_state    = def::terminal_state<U, common_base_tag, Tags...>;
    template < typename U, typename ... Tags >
    using state_machine     = def::state_machine<U, common_base_tag, Tags...>;
};

template < typename T >
struct inner_states_def<T, false> {
    using definition_type   = T;
    using common_base_type  = void;

    template < typename U, typename ... Tags >
    using state             = def::state<U, Tags...>;
    template < typename U, typename ... Tags >
    using terminal_state    = def::terminal_state<U, Tags...>;
    template < typename U, typename ... Tags >
    using state_machine     = def::state_machine<U, Tags...>;
};

}  /* namespace detail */

template < typename T >
struct inner_states_definitions
        : detail::inner_states_def<T, has_common_base<T>::value> {};

}  /* namespace traits */
}  /* namespace def */
}  /* namespace afsm */

#endif /* AFSM_DETAIL_DEF_TRAITS_HPP_ */
