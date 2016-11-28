/*
 * orthogonal_regions.hpp
 *
 *  Created on: 27 нояб. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef AFSM_DETAIL_ORTHOGONAL_REGIONS_HPP_
#define AFSM_DETAIL_ORTHOGONAL_REGIONS_HPP_

#include <afsm/detail/actions.hpp>
#include <afsm/detail/transitions.hpp>

namespace afsm {
namespace orthogonal {

namespace detail {

template < ::std::size_t N >
struct invoke_nth {
    using previous = invoke_nth<N - 1>;
    static constexpr ::std::size_t index = N;
    using event_handler_type = actions::detail::process_event_handler<index>;

    template < typename Regions, typename Event, typename FSM >
    static void
    enter(Regions& regions, Event&& event, FSM& fsm)
    {
        using state_type = typename ::std::tuple_element<index, Regions>::type;
        using state_enter = transitions::detail::state_enter<FSM, state_type, Event>;

        previous::enter(regions, ::std::forward<Event>(event), fsm);
        state_enter{}(::std::get<index>(regions), ::std::forward<Event>(event), fsm);
    }

    template < typename Regions, typename Event, typename FSM >
    static void
    exit(Regions& regions, Event&& event, FSM& fsm)
    {
        using state_type = typename ::std::tuple_element<index, Regions>::type;
        using state_exit = transitions::detail::state_exit<FSM, state_type, Event>;

        // Reverse order of exit
        state_exit{}(::std::get<index>(regions), ::std::forward<Event>(event), fsm);
        previous::exit(regions, ::std::forward<Event>(event), fsm);
    }

    template < typename Regions, typename Event >
    static actions::event_process_result
    process_event(Regions& regions, Event&& event)
    {
        auto res = previous::process_event(regions, ::std::forward<Event>(event));
        return ::std::max(res, event_handler_type{}(regions, ::std::forward<Event>(event)));
    }
};

template <>
struct invoke_nth< 0 > {
    static constexpr ::std::size_t index = 0;
    using event_handler_type = actions::detail::process_event_handler<index>;

    template < typename Regions, typename Event, typename FSM >
    static void
    enter(Regions& regions, Event&& event, FSM& fsm)
    {
        using state_type = typename ::std::tuple_element<index, Regions>::type;
        using state_enter = transitions::detail::state_enter<FSM, state_type, Event>;
        state_enter{}(::std::get<index>(regions), ::std::forward<Event>(event), fsm);
    }

    template < typename Regions, typename Event, typename FSM >
    static void
    exit(Regions& regions, Event&& event, FSM& fsm)
    {
        using state_type = typename ::std::tuple_element<index, Regions>::type;
        using state_exit = transitions::detail::state_exit<FSM, state_type, Event>;
        state_exit{}(::std::get<index>(regions), ::std::forward<Event>(event), fsm);
    }

    template < typename Regions, typename Event >
    static actions::event_process_result
    process_event(Regions& regions, Event&& event)
    {
        return event_handler_type{}(regions, ::std::forward<Event>(event));
    }
};

}  /* namespace detail */

template < typename FSM, typename FSM_DEF, typename Size >
class regions_table {
public:
    using fsm_type                      = FSM;
    using state_machine_definition_type = FSM_DEF;
    using size_type                     = Size;

    using this_type                     = regions_table<FSM, FSM_DEF, Size>;

    using orthogonal_regions            = typename state_machine_definition_type::orthogonal_regions;
    static_assert(!::std::is_same<orthogonal_regions, void>::value,
            "Orthogonal regions table is not defined for orthogonal state machine");

    using regions_def                   = typename def::detail::inner_states< orthogonal_regions >::type;
    using regions_constructor           = ::afsm::detail::front_state_tuple<fsm_type, regions_def>;
    using regions_tuple                 = typename regions_constructor::type;

    static constexpr ::std::size_t size = regions_def::size;

    using region_indexes                = typename ::psst::meta::index_builder<size>::type;
    using all_regions                   = detail::invoke_nth<size - 1>;
public:
    regions_table(fsm_type& fsm)
        : fsm_{&fsm},
          regions_{ regions_constructor::construct(fsm) }
    {}
    regions_table(fsm_type& fsm, regions_table const& rhs)
        : fsm_{&fsm},
          regions_{ regions_constructor::copy_construct(fsm, rhs.regions_) }
    {}
    regions_table(fsm_type& fsm, regions_table&& rhs)
        : fsm_{&fsm},
          regions_{ regions_constructor::move_construct(fsm, ::std::move(rhs.regions_)) }
    {}

    regions_table(regions_table const&) = delete;
    regions_table(regions_table&&) = delete;
    regions_table&
    operator = (regions_table const&) = delete;
    regions_table&
    operator = (regions_table&&) = delete;

    void
    swap(regions_table& rhs)
    {
        using ::std::swap;
        swap(regions_, rhs.regions_);
        ::afsm::detail::set_enclosing_fsm< size - 1 >::set(*fsm_, regions_);
        ::afsm::detail::set_enclosing_fsm< size - 1 >::set(*rhs.fsm_, rhs.regions_);
    }

    regions_tuple&
    regions()
    { return regions_; }
    regions_tuple const&
    regions() const
    { return regions_; }

    template < ::std::size_t N>
    typename ::std::tuple_element< N, regions_tuple >::type&
    get_state()
    { return ::std::get<N>(regions_); }
    template < ::std::size_t N>
    typename ::std::tuple_element< N, regions_tuple >::type const&
    get_state() const
    { return ::std::get<N>(regions_); }

    template < typename Event >
    actions::event_process_result
    process_event(Event&& event)
    {
        // Pass event to all regions
        return all_regions::process_event(regions_, ::std::forward<Event>(event));
    }
    template < typename Event >
    void
    enter(Event&& event)
    {
        // Call enter for all regions
        all_regions::enter(regions_, ::std::forward<Event>(event), *fsm_);
    }
    template < typename Event >
    void
    exit(Event&& event)
    {
        // Call exit for all regions
        all_regions::exit(regions_, ::std::forward<Event>(event), *fsm_);
    }
private:
    fsm_type*           fsm_;
    regions_tuple       regions_;
};

}  /* namespace orthogonal */
}  /* namespace afsm */



#endif /* AFSM_DETAIL_ORTHOGONAL_REGIONS_HPP_ */
