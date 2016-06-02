/*
 * base_states.hpp
 *
 *  Created on: 28 мая 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef AFSM_DETAIL_BASE_STATES_HPP_
#define AFSM_DETAIL_BASE_STATES_HPP_

#include <afsm/definition.hpp>
#include <afsm/detail/helpers.hpp>
#include <afsm/detail/transitions.hpp>

namespace afsm {
namespace detail {

template < actions::event_process_result R >
using process_type = ::std::integral_constant< actions::event_process_result, R >;

template <typename Event, typename HandledEvents,
        typename DeferredEvents = ::psst::meta::type_tuple<>>
struct event_process_selector
    : ::std::conditional<
        ::psst::meta::contains<typename ::std::decay<Event>::type, HandledEvents>::value,
        process_type<actions::event_process_result::process>,
        typename ::std::conditional<
            ::psst::meta::contains<typename ::std::decay<Event>::type, DeferredEvents>::value,
            process_type<actions::event_process_result::defer>,
            process_type<actions::event_process_result::refuse>
        >::type
    >::type{};

template < typename T, bool isTerminal >
struct state_base_impl : T {
    using state_definition_type = T;
    using state_type            = state_base_impl<T, isTerminal>;
    using internal_transitions = typename state_definition_type::internal_transitions;

    static_assert(def::traits::is_state<T>::value,
            "Front state can be created only with a descendant of afsm::def::state");
    static_assert(!def::detail::has_inner_states< internal_transitions >::value,
            "Internal transition table cannot have transitions between other states");
    using handled_events =
            typename def::detail::handled_events<state_definition_type>::type;
    using internal_events =
            typename def::detail::handled_events<state_definition_type>::type;
    static_assert(def::traits::is_state_machine<state_definition_type>::value
                || !def::detail::has_default_transitions< handled_events >::value,
            "Internal transition cannot be a default transition");
    using deferred_events =
            typename ::std::conditional<
                ::std::is_same<typename T::deferred_events, void>::value,
                ::psst::meta::type_tuple<>,
                typename ::psst::meta::unique< typename T::deferred_events >::type
            >::type;

    state_base_impl() : state_definition_type{} {}
    state_base_impl(state_base_impl const&) = default;
    state_base_impl(state_base_impl&&) = default;

    state_base_impl&
    operator = (state_base_impl const&) = default;
    state_base_impl&
    operator = (state_base_impl&&) = default;
protected:
    template< typename ... Args >
    state_base_impl(Args&& ... args)
        : state_definition_type(::std::forward<Args>(args)...) {}
};

template < typename T >
struct state_base_impl<T, true> : T {
    using state_definition_type = T;
    using state_type            = state_base_impl<T, false>;
    using internal_transitions = typename state_definition_type::internal_transitions;

    static_assert(def::traits::is_state<T>::value,
            "Front state can be created only with a descendant of afsm::def::state");
    static_assert(::std::is_same<
            typename state_definition_type::internal_transitions, void >::value,
            "Terminal state must not define transitions");
    static_assert(::std::is_same<
            typename state_definition_type::deferred_events, void >::value,
            "Terminal state must not define deferred events");

    using handled_events  = ::psst::meta::type_tuple<>;
    using internal_events = ::psst::meta::type_tuple<>;
    using deferred_events = ::psst::meta::type_tuple<>;

    state_base_impl() : state_definition_type{} {}
    state_base_impl(state_base_impl const&) = default;
    state_base_impl(state_base_impl&&) = default;

    state_base_impl&
    operator = (state_base_impl const&) = default;
    state_base_impl&
    operator = (state_base_impl&&) = default;
protected:
    template< typename ... Args >
    state_base_impl(Args&& ... args)
        : state_definition_type(::std::forward<Args>(args)...) {}
};

template < typename T >
class state_base : public state_base_impl<T, def::traits::is_terminal_state<T>::value> {
public:
    using state_definition_type = T;
    using state_type            = state_base<T>;
    using internal_transitions  = typename state_definition_type::internal_transitions;
    using base_impl_type        = state_base_impl<T, def::traits::is_terminal_state<T>::value>;
public:
    state_base() : base_impl_type{} {}
    state_base(state_base const&) = default;
    state_base(state_base&&) = default;

    state_base&
    operator = (state_base const&) = default;
    state_base&
    operator = (state_base&&) = default;

    void
    swap(state_base& rhs) noexcept
    {
        using ::std::swap;
        swap(static_cast<T&>(*this), rhs);
    }
protected:
    template< typename ... Args >
    state_base(Args&& ... args)
        : base_impl_type(::std::forward<Args>(args)...) {}
};

template < typename T, typename Mutex, typename FrontMachine >
class state_machine_base_impl : public state_base<T> {
public:
    using state_machine_definition_type = T;
    using state_type                    = state_base<T>;
    using machine_type                  = state_machine_base_impl<T, Mutex, FrontMachine>;
    using front_machine_type            = FrontMachine;
    static_assert(def::traits::is_state_machine<T>::value,
            "Front state machine can be created only with a descendant of afsm::def::state_machine");
    using transitions = typename state_machine_definition_type::transitions;
    using handled_events =
            typename def::detail::recursive_handled_events<state_machine_definition_type>::type;
    static_assert(def::traits::is_state<
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
            ::psst::meta::index_of<initial_state, inner_states_def>::value;
    static constexpr ::std::size_t inner_state_count = inner_states_def::size;

    using state_indexes     = typename ::psst::meta::index_builder<inner_state_count>::type;

    using mutex_type        = Mutex;
    using size_type         = typename detail::size_type<mutex_type>::type;
    using transition_tuple  = afsm::transitions::state_transition_table<
            front_machine_type, state_machine_definition_type, size_type >;

    state_machine_base_impl()
        : state_type{},
          transitions_{front_machine()}
    {}

    state_machine_base_impl(state_machine_base_impl const& rhs)
        : state_type{static_cast<state_type const&>(rhs)},
          transitions_{front_machine(), rhs.transitions_}
    {}
    state_machine_base_impl(state_machine_base_impl&& rhs)
        : state_type{static_cast<state_type&&>(rhs)},
          transitions_{front_machine(), ::std::move(rhs.transitions_)}
    {}

    void
    swap(state_machine_base_impl& rhs) noexcept
    {
        using ::std::swap;
        swap(static_cast<state_type&>(*this), rhs);
        swap(transitions_, rhs.transitions_);
    }

    state_machine_base_impl&
    operator = (state_machine_base_impl const& rhs)
    {
        using ::std::swap;
        state_machine_base_impl tmp{rhs};
        swap(*this, tmp);
        return *this;
    }
    state_machine_base_impl&
    operator = (state_machine_base_impl&& rhs)
    {
        using ::std::swap;
        swap(*this, rhs);
        return *this;
    }

    template < ::std::size_t N>
    ::std::tuple_element< N, inner_states_tuple >&
    get_state()
    { return transitions_.template get_state<N>(); }
    template < ::std::size_t N>
    ::std::tuple_element< N, inner_states_tuple > const&
    get_state() const
    { return transitions_.template get_state<N>(); }

    ::std::size_t
    current_state() const
    { return transitions_.current_state(); }

    front_machine_type&
    front_machine()
    {
        return static_cast<front_machine_type&>(*this);
    }

    template < typename Event >
    void
    state_enter(Event&& event)
    {
        transitions_.enter( ::std::forward<Event>(event) );
    }
    template < typename Event >
    void
    state_exit(Event&& event)
    {
        transitions_.exit( ::std::forward<Event>(event) );
    }
protected:
    template<typename ... Args>
    explicit
    state_machine_base_impl(Args&& ... args)
        : state_type(::std::forward<Args>(args)...),
          transitions_{front_machine()}
    {}

    template < typename FSM, typename Event >
    actions::event_process_result
    process_event_impl(FSM& enclosing_fsm, Event&& event,
        detail::process_type<actions::event_process_result::process> const&)
    {
        // Transitions and internal event dispatch
        auto res = transitions_.process_event(::std::forward<Event>(event));
        if (res == actions::event_process_result::refuse) {
            // Internal transitions
            res = actions::handle_in_state_event(::std::forward<Event>(event), enclosing_fsm, *this);
        }
        return res;
    }
    template < typename FSM, typename Event >
    actions::event_process_result
    process_event_impl(FSM&, Event&&,
        detail::process_type<actions::event_process_result::defer> const&)
    {
        return actions::event_process_result::defer;
    }
    template < typename FSM, typename Event >
    actions::event_process_result
    process_event_impl(FSM&, Event&&,
        detail::process_type<actions::event_process_result::refuse> const&)
    {
        return actions::event_process_result::refuse;
    }
protected:
    transition_tuple        transitions_;
};

template < typename T, typename Mutex, typename FrontMachine >
constexpr ::std::size_t state_machine_base_impl<T, Mutex, FrontMachine>::initial_state_index;
template < typename T, typename Mutex, typename FrontMachine >
constexpr ::std::size_t state_machine_base_impl<T, Mutex, FrontMachine>::inner_state_count;

template < typename T, typename Mutex, typename FrontMachine >
struct state_machine_base_with_base : state_machine_base_impl<T, Mutex, FrontMachine> {
    using base_type = state_machine_base_impl<T, Mutex, FrontMachine>;
    using common_base = typename T::common_base;

    state_machine_base_with_base() = default;
    state_machine_base_with_base(state_machine_base_with_base const&) = default;
    state_machine_base_with_base(state_machine_base_with_base&&) = default;

    state_machine_base_with_base&
    operator = (state_machine_base_with_base const&) = default;
    state_machine_base_with_base&
    operator = (state_machine_base_with_base&&) = default;

    common_base&
    current_state_base()
    {
        return base_type::transitions_.template cast_current_state<common_base>();
    }
    common_base const&
    current_state_base() const
    {
        return base_type::transitions_.template cast_current_state<common_base const>();
    }
protected:
    template<typename ... Args>
    explicit
    state_machine_base_with_base(Args&& ... args)
        : state_machine_base_with_base::machine_type(::std::forward<Args>(args)...)
    {}
};

template < typename T, typename Mutex, typename FrontMachine >
struct state_machine_base : ::std::conditional<
        def::traits::has_common_base<T>::value,
        state_machine_base_with_base< T, Mutex, FrontMachine >,
        state_machine_base_impl< T, Mutex, FrontMachine >
    >::type {
public:
    using state_machine_impl_type = typename ::std::conditional<
            def::traits::has_common_base<T>::value,
            state_machine_base_with_base< T, Mutex, FrontMachine >,
            state_machine_base_impl< T, Mutex, FrontMachine >
        >::type;
    state_machine_base() = default;
    state_machine_base(state_machine_base const&) = default;
    state_machine_base(state_machine_base&&) = default;

    state_machine_base&
    operator = (state_machine_base const&) = default;
    state_machine_base&
    operator = (state_machine_base&&) = default;
protected:
    template<typename ... Args>
    explicit
    state_machine_base(Args&& ... args)
        : state_machine_impl_type(::std::forward<Args>(args)...)
    {}
};

}  /* namespace detail */
}  /* namespace afsm */



#endif /* AFSM_DETAIL_BASE_STATES_HPP_ */
