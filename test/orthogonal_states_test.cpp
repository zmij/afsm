/*
 * orthogonal_states_test.cpp
 *
 *  Created on: Nov 28, 2016
 *      Author: zmij
 */

#include <afsm/fsm.hpp>

#include <gtest/gtest.h>

namespace afsm {
namespace test {

namespace events {

struct power_on {};
struct power_off {};

struct do_work {};
struct error {};

} /* namespace events */

struct ortho_sm_def : def::state_machine<ortho_sm_def> {
    struct off : state<off> {};
    struct on : state_machine<on> {
        struct work : state_machine<work> {
            struct state_a : state<state_a> {};
            struct state_b : state<state_b> {};
            using initial_state = state_a;

            // clang-format off
            using transitions = transition_table<
                tr< state_a,    events::do_work,    state_b >,
                tr< state_b,    events::do_work,    state_a >
            >;
            // clang-format on
        };
        struct error : state_machine<error> {
            struct no : state<no> {};
            struct yes : state<yes> {
                using internal_transitions = transition_table<in<events::error>>;
            };
            using initial_state = no;
            using transitions   = transition_table<tr<no, events::error, yes>>;
        };
        using orthogonal_regions = type_tuple<work, error>;
    };

    using initial_state = off;
    // clang-format off
    using transitions = transition_table<
        tr< off,    events::power_on,   on  >,
        tr< on,     events::power_off,  off >
    >;
    // clang-format on
};

static_assert(def::traits::has_orthogonal_regions<ortho_sm_def::on>::value, "");
static_assert(def::traits::is_state_machine<ortho_sm_def::on>::value, "");

using work_fsm = state_machine<ortho_sm_def::on>;
static_assert(!::std::is_same<work_fsm::orthogonal_regions, void>::value, "");
static_assert(!::std::is_same<work_fsm::regions_def, void>::value, "");
static_assert(::psst::meta::contains<ortho_sm_def::on::work, work_fsm::regions_def>::value, "");
static_assert(::psst::meta::contains<ortho_sm_def::on::error, work_fsm::regions_def>::value, "");

using work_inner_fsm = inner_state_machine<ortho_sm_def::on, state_machine<ortho_sm_def>>;
static_assert(!::std::is_same<work_inner_fsm::orthogonal_regions, void>::value, "");
static_assert(!::std::is_same<work_inner_fsm::regions_def, void>::value, "");
static_assert(::psst::meta::contains<work_inner_fsm::on::work, work_inner_fsm::regions_def>::value,
              "");
static_assert(::psst::meta::contains<work_inner_fsm::on::error, work_inner_fsm::regions_def>::value,
              "");

using ortho_fsm = state_machine<ortho_sm_def>;
static_assert(def::contains_substate<ortho_fsm, ortho_fsm::on>::value, "");
static_assert(def::contains_substate<ortho_fsm, ortho_fsm::on::work>::value, "");
static_assert(def::contains_substate<ortho_fsm, ortho_fsm::on::error>::value, "");

TEST(OrthogonalRegions, Simple)
{
    ortho_fsm fsm;
    EXPECT_TRUE(done(fsm.process_event(events::power_on{})));
    EXPECT_TRUE(fsm.is_in_state<ortho_fsm::on>());
    EXPECT_TRUE(fsm.is_in_state<ortho_fsm::on::work>());
    EXPECT_TRUE(fsm.is_in_state<ortho_fsm::on::work::state_a>());
    EXPECT_TRUE(fsm.is_in_state<ortho_fsm::on::error>());
    EXPECT_TRUE(fsm.is_in_state<ortho_fsm::on::error::no>());

    EXPECT_TRUE(done(fsm.process_event(events::do_work{})));
    EXPECT_TRUE(done(fsm.process_event(events::error{})));
    EXPECT_TRUE(fsm.is_in_state<ortho_fsm::on>());
    EXPECT_TRUE(fsm.is_in_state<ortho_fsm::on::work>());
    EXPECT_TRUE(fsm.is_in_state<ortho_fsm::on::work::state_b>());
    EXPECT_TRUE(fsm.is_in_state<ortho_fsm::on::error>());
    EXPECT_TRUE(fsm.is_in_state<ortho_fsm::on::error::yes>());

    EXPECT_TRUE(done(fsm.process_event(events::do_work{})));
    EXPECT_TRUE(done(fsm.process_event(events::error{})));
    EXPECT_TRUE(fsm.is_in_state<ortho_fsm::on>());
    EXPECT_TRUE(fsm.is_in_state<ortho_fsm::on::work>());
    EXPECT_TRUE(fsm.is_in_state<ortho_fsm::on::work::state_a>());
    EXPECT_TRUE(fsm.is_in_state<ortho_fsm::on::error>());
    EXPECT_TRUE(fsm.is_in_state<ortho_fsm::on::error::yes>());
}

} /* namespace test */
} /* namespace afsm */
