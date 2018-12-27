/*
 * transaction_sm.cpp
 *
 *  Created on: May 27, 2016
 *      Author: zmij
 */

#include "transaction_common.hpp"
#include <afsm/fsm.hpp>

#include <gtest/gtest.h>

namespace afsm {
namespace test {

struct connection_pri_fsm_def
    : def::state_machine<connection_pri_fsm_def, def::tags::common_base<state_name>> {
    using connection_fsm
        = ::afsm::priority_state_machine<connection_pri_fsm_def, ::std::mutex, test_fsm_observer>;

    connection_fsm&
    fsm()
    {
        return static_cast<connection_fsm&>(*this);
    }
    connection_fsm const&
    fsm() const
    {
        return static_cast<connection_fsm const&>(*this);
    }
    ::std::string
    name() const override
    {
        return fsm().current_state_base().name();
    }
    struct closed : state<closed> {
        ::std::string
        name() const override
        {
            return "closed";
        }
    };

    struct connecting : state<connecting> {
        ::std::string
        name() const override
        {
            return "connecting";
        }
    };

    struct authorizing : state<authorizing> {
        ::std::string
        name() const override
        {
            return "authorizing";
        }
    };

    struct idle : state<idle> {
        ::std::string
        name() const override
        {
            return "idle";
        }
    };

    struct terminated : terminal_state<terminated> {
        ::std::string
        name() const override
        {
            return "teminated";
        }
    };

    struct transaction : state_machine<transaction> {
        using transaction_fsm = ::afsm::inner_state_machine<transaction, connection_fsm>;

        transaction() {}
        ::std::string
        name() const override
        {
            return "transaction " + fsm().current_state_base().name();
        }
        transaction_fsm&
        fsm()
        {
            return static_cast<transaction_fsm&>(*this);
        }
        transaction_fsm const&
        fsm() const
        {
            return static_cast<transaction_fsm const&>(*this);
        }

        struct starting : state<starting> {
            using deferred_events = type_tuple<events::execute, events::exec_prepared,
                                               events::commit, events::rollback>;
            ::std::string
            name() const override
            {
                return "starting";
            }
            // clang-format off
            using internal_transitions = transition_table<
                in< events::command_complete,   transit_action,   none >
            >;
            // clang-format on
        };

        struct idle : state<idle> {
            ::std::string
            name() const override
            {
                return "transaction idle";
            }
            // clang-format off
            using internal_transitions = transition_table<
                in< events::command_complete,   transit_action,   none >,
                in< events::ready_for_query,    transit_action,   none >
            >;
            // clang-format on
        };

        struct simple_query : state_machine<simple_query> {
            using simple_query_fsm = ::afsm::inner_state_machine<simple_query, transaction_fsm>;
            ::std::string
            name() const override
            {
                return "simple query " + fsm().current_state_base().name();
            }
            simple_query_fsm&
            fsm()
            {
                return static_cast<simple_query_fsm&>(*this);
            }
            simple_query_fsm const&
            fsm() const
            {
                return static_cast<simple_query_fsm const&>(*this);
            }

            struct waiting : state<waiting> {
                ::std::string
                name() const override
                {
                    return "waiting results";
                }
                // clang-format off
                using internal_transitions = transition_table<
                    in< events::command_complete,   transit_action >
                >;
                // clang-format on
            };
            struct fetch_data : state<fetch_data> {
                ::std::string
                name() const override
                {
                    return "fetch data";
                }
                using internal_transitions
                    = transition_table<in<events::row_event, transit_action>>;
            };

            using initial_state = waiting;
            // clang-format off
            using transitions = transition_table<
                tr< waiting,    events::row_description,    fetch_data,     transit_action>,
                tr< fetch_data, events::command_complete,   waiting,        transit_action>
            >;
            using deferred_events = type_tuple<
                events::execute,
                events::exec_prepared,
                events::commit,
                events::rollback
            >;
            // clang-format on
        };

        struct extended_query : state_machine<extended_query> {
            using extended_query_fsm = ::afsm::inner_state_machine<extended_query, transaction_fsm>;
            ::std::string
            name() const override
            {
                return "extended query " + fsm().current_state_base().name();
            }
            extended_query_fsm&
            fsm()
            {
                return static_cast<extended_query_fsm&>(*this);
            }
            extended_query_fsm const&
            fsm() const
            {
                return static_cast<extended_query_fsm const&>(*this);
            }

            struct prepare : state<prepare> {
                ::std::string
                name() const override
                {
                    return "prepare";
                }
            };
            struct parse : state<parse> {
                ::std::string
                name() const override
                {
                    return "parse";
                }
                // clang-format off
                using internal_transitions = transition_table<
                    in< events::row_description,    transit_action >,
                    in< events::no_data,            transit_action >
                >;
                // clang-format on
            };
            struct bind : state<bind> {
                ::std::string
                name() const override
                {
                    return "bind";
                }
            };
            struct exec : state<exec> {
                ::std::string
                name() const override
                {
                    return "exec";
                }
                // clang-format off
                using internal_transitions = transition_table<
                    in< events::row_event,          transit_action  >,
                    in< events::command_complete,   transit_action  >
                >;
                // clang-format on
            };

            using initial_state = prepare;
            // clang-format off
            using transitions = transition_table<
                tr< prepare,    none,                       parse,      transit_action >,
                tr< parse,      events::ready_for_query,    bind,       transit_action >,
                tr< bind,       events::ready_for_query,    exec,       transit_action >
            >;
            // clang-format on
        };

        struct tran_error : state<tran_error> {
            ::std::string
            name() const override
            {
                return "tran error";
            }
            using internal_transitions = transition_table<in<events::commit>, in<events::rollback>>;
        };

        struct exiting : state<exiting> {
            ::std::string
            name() const override
            {
                return "exiting";
            }
            // clang-format off
            using internal_transitions = transition_table<
                in< events::command_complete,   transit_action >,
                in< events::commit,             transit_action >,
                in< events::rollback,           transit_action >,
                in< events::execute,            transit_action >,
                in< events::exec_prepared,      transit_action >
            >;
            // clang-format on
        };

        using initial_state = starting;
        // clang-format off
        using transitions = transition_table<
            tr< starting,       events::ready_for_query,    idle,           transit_action>,

            tr< idle,           events::commit,             exiting,        transit_action         >,
            tr< idle,           events::rollback,           exiting,        transit_action>,
            tr< idle,           events::query_error,        exiting,        transit_action>,
            tr< idle,           events::client_error,       exiting,        transit_action>,

            tr< idle,           events::execute,            simple_query,   transit_action>,
            tr< simple_query,   events::ready_for_query,    idle,           transit_action>,
            tr< simple_query,   events::query_error,        tran_error,     transit_action>,
            tr< simple_query,   events::client_error,       tran_error,     transit_action>,

            tr< idle,           events::exec_prepared,      extended_query, transit_action>,
            tr< extended_query, events::ready_for_query,    idle,           transit_action>,
            tr< extended_query, events::query_error,        tran_error,     transit_action>,
            tr< extended_query, events::client_error,       tran_error,     transit_action>,

            tr< tran_error,     events::ready_for_query,    exiting,        transit_action>
        >;
        // clang-format on
    };

    using initial_state = closed;
    // clang-format off
    using transitions = transition_table<
        tr< closed,             events::connect,            connecting,     transit_action>,
        tr< closed,             events::terminate,          terminated,     transit_action>,

        tr< connecting,         events::complete,           authorizing,    transit_action>,
        tr< connecting,         events::conn_error,         terminated,     transit_action>,

        tr< authorizing,        events::ready_for_query,    idle,           transit_action>,
        tr< authorizing,        events::conn_error,         terminated,     transit_action>,

        tr< idle,               events::begin,              transaction,    transit_action>,
        tr< idle,               events::conn_error,         terminated,     transit_action>,
        tr< idle,               events::terminate,          terminated,     transit_action>,

        tr< transaction,        events::ready_for_query,    idle,           transit_action>,
        tr< transaction,        events::conn_error,         terminated,     transit_action>
    >;
    // clang-format off
};

using connection_fsm = priority_state_machine<connection_pri_fsm_def, ::std::mutex, test_fsm_observer>;

static_assert(def::contains_substate<connection_pri_fsm_def, connection_pri_fsm_def::connecting>::value, "");
static_assert(def::contains_substate<connection_pri_fsm_def, connection_pri_fsm_def::transaction>::value, "");
static_assert(def::contains_substate<connection_pri_fsm_def::transaction, connection_pri_fsm_def::transaction::idle>::value, "");
static_assert(!def::contains_substate<connection_pri_fsm_def::transaction, connection_pri_fsm_def::connecting>::value, "");
static_assert(def::contains_substate<connection_pri_fsm_def, connection_pri_fsm_def::transaction::idle>::value, "");

static_assert(
    ::std::is_base_of< detail::containment_type< detail::state_containment::immediate >,
     detail::state_containment_type<
             connection_pri_fsm_def::connecting,
             connection_pri_fsm_def,
             connection_fsm::inner_states_def>>::value, "");

static_assert(
    detail::state_containment_type<
        connection_pri_fsm_def::connecting,
        connection_pri_fsm_def,
        connection_fsm::inner_states_def>::value == detail::state_containment::immediate, "");
static_assert(
    detail::state_containment_type<
        connection_pri_fsm_def::transaction::idle,
        connection_pri_fsm_def,
        connection_fsm::inner_states_def>::value == detail::state_containment::substate, "");
static_assert(
    detail::state_containment_type<
        connection_pri_fsm_def,
        connection_pri_fsm_def,
        connection_fsm::inner_states_def>::value == detail::state_containment::self, "");

namespace {
void
begin_transaction(connection_fsm& fsm)
{
    using actions::event_process_result;
    using ::psst::ansi_color;
    ::std::cout << ansi_color::yellow << ::std::setw(80) << ::std::setfill('=') << '='
            << "\n";
    // Start transaction sequence
    EXPECT_EQ(event_process_result::process, fsm.process_event(events::begin{}));
    EXPECT_EQ(event_process_result::process, fsm.process_event(events::ready_for_query{}));

    EXPECT_TRUE(fsm.is_in_state<connection_pri_fsm_def::transaction>());
    EXPECT_TRUE(fsm.is_in_state<connection_pri_fsm_def::transaction::idle>());
}

void
commit_transaction(connection_fsm& fsm)
{
    using actions::event_process_result;
    using ::psst::ansi_color;
    // Commit transaction sequence
    EXPECT_EQ(event_process_result::process, fsm.process_event(events::commit{}));
    EXPECT_EQ(event_process_result::process_in_state, fsm.process_event(events::command_complete{}));
    EXPECT_EQ(event_process_result::process, fsm.process_event(events::ready_for_query{}));
    ::std::cout << ansi_color::yellow << ::std::setw(80) << ::std::setfill('=') << '='
            << "\n";
    EXPECT_FALSE(fsm.is_in_state<connection_pri_fsm_def::transaction>());
    EXPECT_FALSE(fsm.is_in_state<connection_pri_fsm_def::transaction::idle>());
}
}  /* namespace  */

TEST(TranPriorityFSM, AllEvents)
{
    using actions::event_process_result;
    using ::psst::ansi_color;
    connection_fsm fsm;
    fsm.make_observer("connection_pri_fsm_def");

    EXPECT_EQ(event_process_result::process, fsm.process_event(events::connect{}));
    EXPECT_TRUE(fsm.is_in_state<connection_pri_fsm_def::connecting>());
    EXPECT_EQ(event_process_result::process, fsm.process_event(events::complete{}));
    EXPECT_FALSE(fsm.is_in_state<connection_pri_fsm_def::connecting>());
    EXPECT_TRUE(fsm.is_in_state<connection_pri_fsm_def::authorizing>());
    EXPECT_FALSE(fsm.is_in_state<connection_pri_fsm_def::idle>());
    EXPECT_EQ(event_process_result::process, fsm.process_event(events::ready_for_query{}));
    EXPECT_TRUE(fsm.is_in_state<connection_pri_fsm_def::idle>());

    begin_transaction(fsm);

    // Simple query sequence
    EXPECT_EQ(event_process_result::process, fsm.process_event(events::execute{}));
    EXPECT_TRUE(fsm.is_in_state<connection_pri_fsm_def::transaction::simple_query>());
    EXPECT_TRUE(fsm.is_in_state<connection_pri_fsm_def::transaction::simple_query::waiting>());
    EXPECT_EQ(event_process_result::process, fsm.process_event(events::row_description{}));
    EXPECT_TRUE(fsm.is_in_state<connection_pri_fsm_def::transaction::simple_query>());
    EXPECT_TRUE(fsm.is_in_state<connection_pri_fsm_def::transaction::simple_query::fetch_data>());
    EXPECT_EQ(event_process_result::process_in_state, fsm.process_event(events::row_event{}));
    EXPECT_EQ(event_process_result::process_in_state, fsm.process_event(events::row_event{}));
    EXPECT_EQ(event_process_result::process, fsm.process_event(events::command_complete{}));
    EXPECT_EQ(event_process_result::process, fsm.process_event(events::ready_for_query{}));

    commit_transaction(fsm);

    begin_transaction(fsm);

    // Extended query no data sequence
    EXPECT_EQ(event_process_result::process, fsm.process_event(events::exec_prepared{}));
    // parse -> parse
    EXPECT_EQ(event_process_result::process_in_state, fsm.process_event(events::no_data{}));
    // parse -> bind
    EXPECT_EQ(event_process_result::process, fsm.process_event(events::ready_for_query{}));
    // bind -> exec
    EXPECT_EQ(event_process_result::process, fsm.process_event(events::ready_for_query{}));
    // exec -> idle
    EXPECT_EQ(event_process_result::process, fsm.process_event(events::ready_for_query{}));

    commit_transaction(fsm);

    // Terminate connection
    EXPECT_EQ(event_process_result::process, fsm.process_event(events::terminate{}));
}

TEST(TranPriorityFSM, RealEventSequence)
{
    using actions::event_process_result;
    using ::psst::ansi_color;
    connection_fsm fsm;
    fsm.make_observer("connection_pri_fsm_def");

    // Enqueueing commands
    EXPECT_EQ(event_process_result::process, fsm.process_event(events::connect{}));
    EXPECT_EQ(event_process_result::process, fsm.process_event(events::complete{}));
    // Connect response
    EXPECT_EQ(event_process_result::process, fsm.process_event(events::ready_for_query{}));

    // Enqueueing commands
    EXPECT_EQ(event_process_result::process, fsm.process_event(events::begin{}));
    EXPECT_EQ(event_process_result::defer, fsm.process_event(events::execute{}));
    EXPECT_EQ(event_process_result::defer, fsm.process_event(events::commit{}));

    // Server responses
    // Begin
    EXPECT_EQ(event_process_result::process_in_state, fsm.process_event(events::command_complete{}));
    EXPECT_EQ(event_process_result::process, fsm.process_event(events::ready_for_query{}));
    // Simple query responses
    EXPECT_EQ(event_process_result::process, fsm.process_event(events::row_description{}));
    EXPECT_EQ(event_process_result::process_in_state, fsm.process_event(events::row_event{}));
    EXPECT_EQ(event_process_result::process_in_state, fsm.process_event(events::row_event{}));
    EXPECT_EQ(event_process_result::process, fsm.process_event(events::command_complete{}));
    EXPECT_EQ(event_process_result::process, fsm.process_event(events::ready_for_query{}));
    // Commit response
    EXPECT_EQ(event_process_result::process, fsm.process_event(events::ready_for_query{}));
}

}  /* namespace test */
}  /* namespace afsm */

