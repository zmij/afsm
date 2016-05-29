/*
 * type_tuple.hpp
 *
 *  Created on: 28 мая 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef AFSM_META_TYPE_TUPLE_HPP_
#define AFSM_META_TYPE_TUPLE_HPP_

#include <afsm/meta/nth_type.hpp>

namespace afsm {
namespace meta {

template < typename ... T >
struct type_tuple {
    static constexpr ::std::size_t size = sizeof ... (T);
    template < ::std::size_t N >
    using type = typename nth_type<N, T ...>::type;
};

template <>
struct type_tuple<> {
    static constexpr ::std::size_t size = 0;
};

}  /* namespace meta */
}  /* namespace afsm */



#endif /* AFSM_META_TYPE_TUPLE_HPP_ */
