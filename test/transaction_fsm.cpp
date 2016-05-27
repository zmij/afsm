/*
 * transaction_sm.cpp
 *
 *  Created on: May 27, 2016
 *      Author: zmij
 */

#include <gtest/gtest.h>
#include <afsm/fsm.hpp>


namespace afsm {
namespace test {

namespace events {

struct connect{};
struct complete{};
struct ready_for_query{};
struct begin{};
struct terminate{};

struct execute{};
struct exec_prepared{};

struct commit{};
struct rollback{};

struct conn_error{};
struct query_error{};
struct client_error{};

struct row_description {};
struct command_complete {};

}  /* namespace events */

struct connection_fsm_def : def::state_machine<connection_fsm_def> {
    struct closed : def::state<closed> {
    };

    struct connecting : def::state<connecting> {
    };

    struct authorizing : def::state<authorizing> {
    };

    struct idle : def::state<idle> {
    };

    struct terminated : def::state<terminated> {
    };

    struct transaction : def::state_machine<transaction> {
        struct starting : def::state<starting> {
        };

        struct idle : def::state<idle> {
        };

        struct simple_query : def::state_machine<simple_query> {
            struct waiting : def::state<waiting> {};
            struct fetch_data : def::state<fetch_data>{};

            using initial_state = waiting;
            using transitions = def::transition_table<
                tr< waiting,    events::row_description,    fetch_data      >,
                tr< fetch_data, events::command_complete,   waiting         >
            >;
        };

        struct extended_query : def::state_machine<extended_query> {
            struct prepare  : def::state<prepare> {};
            struct parse    : def::state<parse> {};
            struct bind     : def::state<bind> {};
            struct exec     : def::state<exec> {};

            using initial_state = prepare;
            using transitions = def::transition_table<
                tr< prepare,    none,                       parse           >,
                tr< parse,      events::ready_for_query,    bind            >,
                tr< bind,       events::ready_for_query,    exec            >
            >;
        };

        struct tran_error : def::state<tran_error> {
        };

        struct exiting : def::state<exiting> {
        };

        using initial_state = starting;
        using transitions = def::transition_table<
            tr< starting,   events::ready_for_query,    idle                >,

            tr< idle,           events::commit,             exiting         >,
            tr< idle,           events::rollback,           exiting         >,
            tr< idle,           events::query_error,        exiting         >,
            tr< idle,           events::client_error,       exiting         >,

            tr< idle,           events::execute,            simple_query    >,
            tr< simple_query,   events::ready_for_query,    idle            >,
            tr< simple_query,   events::query_error,        tran_error      >,
            tr< simple_query,   events::client_error,       tran_error      >,

            tr< idle,           events::exec_prepared,      extended_query  >,
            tr< extended_query, events::ready_for_query,    idle            >,
            tr< extended_query, events::query_error,        tran_error      >,
            tr< extended_query, events::client_error,       tran_error      >,

            tr< tran_error,     events::ready_for_query,    exiting         >
        >;
    };

    using initial_state = closed;
    using transitions = def::transition_table<
        tr< closed,             events::connect,            connecting      >,
        tr< closed,             events::terminate,          terminated      >,

        tr< connecting,         events::complete,           authorizing     >,
        tr< connecting,         events::conn_error,         terminated      >,

        tr< authorizing,        events::ready_for_query,    idle            >,
        tr< authorizing,        events::conn_error,         terminated      >,

        tr< idle,               events::begin,              transaction     >,
        tr< idle,               events::conn_error,         terminated      >,
        tr< idle,               events::terminate,          terminated      >,

        tr< transaction,        events::ready_for_query,    idle            >,
        tr< transaction,        events::conn_error,         terminated      >
    >;
};

using connection_fsm = state_machine<connection_fsm_def>;

TEST(TranFSM, Transitions)
{
    connection_fsm fsm;
}

}  /* namespace test */
}  /* namespace afsm */

