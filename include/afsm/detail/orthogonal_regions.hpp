/*
 * orthogonal_regions.hpp
 *
 *  Created on: 27 нояб. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef AFSM_DETAIL_ORTHOGONAL_REGIONS_HPP_
#define AFSM_DETAIL_ORTHOGONAL_REGIONS_HPP_

#include <afsm/detail/actions.hpp>

namespace afsm {
namespace orthogonal {

namespace detail {

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
    process_event(Event&&)
    {
        // Pass event to all regions
        return actions::event_process_result::refuse;
    }
    template < typename Event >
    void
    enter(Event&&)
    {
        // Call enter for all regions
    }
    template < typename Event >
    void
    exit(Event&&)
    {
        // Call exit for all regions
    }
private:
    fsm_type*           fsm_;
    regions_tuple       regions_;
};

}  /* namespace orthogonal */
}  /* namespace afsm */



#endif /* AFSM_DETAIL_ORTHOGONAL_REGIONS_HPP_ */
