/*
 * tags.hpp
 *
 *  Created on: 1 июня 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef AFSM_DETAIL_TAGS_HPP_
#define AFSM_DETAIL_TAGS_HPP_

namespace afsm {
namespace def {
namespace tags {

struct state {};
struct state_machine{};

struct has_history {};
struct has_common_base {};

struct allow_empty_enter_exit {};
struct mandatory_empty_enter_exit {};

}  /* namespace tags */
}  /* namespace def */
}  /* namespace afsm */

#endif /* AFSM_DETAIL_TAGS_HPP_ */
