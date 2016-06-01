/*
 * transaction_sm.cpp
 *
 *  Created on: May 27, 2016
 *      Author: zmij
 */

#include <gtest/gtest.h>
#include <afsm/fsm.hpp>
#include <iostream>

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
struct no_data{};
struct row_event{};
struct command_complete {};

}  /* namespace events */

struct dummy_action {
    template < typename Event, typename FSM, typename SourceState, typename TargetState >
    void
    operator()(Event&&, FSM&, SourceState&, TargetState&) const
    {
        ::std::cerr << "Dummy action triggered\n";
    }
};

struct connection_fsm_def : def::state_machine<connection_fsm_def> {
    struct closed : state<closed> {
        template <typename Event, typename FSM>
        void
        on_exit(Event&&, FSM&)
        {
            ::std::cerr << "Exit closed\n";
        }
    };

    struct connecting : state<connecting> {
        template <typename Event, typename FSM>
        void
        on_enter(Event&&, FSM&)
        {
            ::std::cerr << "Enter connecting\n";
        }
        template <typename Event, typename FSM>
        void
        on_exit(Event&&, FSM&)
        {
            ::std::cerr << "Exit connecting\n";
        }
    };

    struct authorizing : state<authorizing> {
        template <typename Event, typename FSM>
        void
        on_enter(Event&&, FSM&)
        {
            ::std::cerr << "Enter authorizing\n";
        }
        template <typename Event, typename FSM>
        void
        on_exit(Event&&, FSM&)
        {
            ::std::cerr << "Exit authorizing\n";
        }
    };

    struct idle : state<idle> {
        template <typename Event, typename FSM>
        void
        on_enter(Event&&, FSM&)
        {
            ::std::cerr << "Enter idle\n";
        }
        template <typename Event, typename FSM>
        void
        on_exit(Event&&, FSM&)
        {
            ::std::cerr << "Exit idle\n";
        }
    };

    struct terminated : terminal_state<terminated> {
        template <typename Event, typename FSM>
        void
        on_enter(Event&&, FSM&)
        {
            ::std::cerr << "Enter terminated\n";
        }
    };

    struct transaction : state_machine<transaction> {
        template <typename Event, typename FSM>
        void
        on_enter(Event&&, FSM&)
        {
            ::std::cerr << "Enter transaction\n";
        }
        template <typename Event, typename FSM>
        void
        on_exit(Event&&, FSM&)
        {
            ::std::cerr << "Exit transaction\n";
        }
        struct starting : state<starting> {
            template <typename Event, typename FSM>
            void
            on_enter(Event&&, FSM&)
            {
                ::std::cerr << "Enter transaction starting\n";
            }
            template <typename Event, typename FSM>
            void
            on_exit(Event&&, FSM&)
            {
                ::std::cerr << "Exit transaction starting\n";
            }
            using internal_transitions = transition_table<
                in< events::command_complete >
            >;
        };

        struct idle : state<idle> {
            template <typename Event, typename FSM>
            void
            on_enter(Event&&, FSM&)
            {
                ::std::cerr << "Enter transaction idle\n";
            }
            template <typename Event, typename FSM>
            void
            on_exit(Event&&, FSM&)
            {
                ::std::cerr << "Exit transaction idle\n";
            }
            using internal_transitions = transition_table<
                in< events::command_complete,   dummy_action,   none >,
                in< events::ready_for_query,    dummy_action,   none >
            >;
        };

        struct simple_query : state_machine<simple_query> {
            template <typename Event, typename FSM>
            void
            on_enter(Event&&, FSM&)
            {
                ::std::cerr << "Enter simple query\n";
            }
            template <typename Event, typename FSM>
            void
            on_exit(Event&&, FSM&)
            {
                ::std::cerr << "Exit simple query\n";
            }
            struct waiting : state<waiting> {
                template <typename Event, typename FSM>
                void
                on_enter(Event&&, FSM&)
                {
                    ::std::cerr << "Enter waiting\n";
                }
                template <typename Event, typename FSM>
                void
                on_exit(Event&&, FSM&)
                {
                    ::std::cerr << "Exit waiting\n";
                }
                using internal_transitions = transition_table<
                    in< events::command_complete >
                >;
            };
            struct fetch_data : state<fetch_data>{
                template <typename Event, typename FSM>
                void
                on_enter(Event&&, FSM&)
                {
                    ::std::cerr << "Enter fetch data\n";
                }
                template <typename Event, typename FSM>
                void
                on_exit(Event&&, FSM&)
                {
                    ::std::cerr << "Exit fetch data\n";
                }
                using internal_transitions = transition_table<
                    in< events::row_event >
                >;
            };

            using initial_state = waiting;
            using transitions = transition_table<
                tr< waiting,    events::row_description,    fetch_data      >,
                tr< fetch_data, events::command_complete,   waiting         >
            >;
        };

        struct extended_query : state_machine<extended_query> {
            template <typename Event, typename FSM>
            void
            on_enter(Event&&, FSM&)
            {
                ::std::cerr << "Enter extended query\n";
            }
            template <typename Event, typename FSM>
            void
            on_exit(Event&&, FSM&)
            {
                ::std::cerr << "Exit extended query\n";
            }
            struct prepare  : state<prepare> {};
            struct parse    : state<parse> {
                using internal_transitions = transition_table<
                    in< events::row_description >,
                    in< events::no_data >
                >;
            };
            struct bind     : state<bind> {};
            struct exec     : state<exec> {
                using internal_transitions = transition_table<
                    in< events::row_event >,
                    in< events::command_complete >
                >;
            };

            using initial_state = prepare;
            using transitions = transition_table<
                tr< prepare,    none,                       parse           >,
                tr< parse,      events::ready_for_query,    bind            >,
                tr< bind,       events::ready_for_query,    exec            >
            >;
        };

        struct tran_error : state<tran_error> {
            template <typename Event, typename FSM>
            void
            on_enter(Event&&, FSM&)
            {
                ::std::cerr << "Enter transaction error\n";
            }
            template <typename Event, typename FSM>
            void
            on_exit(Event&&, FSM&)
            {
                ::std::cerr << "Exit transaction error\n";
            }
            using internal_transitions = transition_table<
                in< events::commit >,
                in< events::rollback >
            >;
        };

        struct exiting : state<exiting> {
            template <typename Event, typename FSM>
            void
            on_enter(Event&&, FSM&)
            {
                ::std::cerr << "Enter transaction exiting\n";
            }
            template <typename Event, typename FSM>
            void
            on_exit(Event&&, FSM&)
            {
                ::std::cerr << "Exit transaction exiting\n";
            }
            using internal_transition = transition_table<
                in< events::command_complete >,
                in< events::commit >,
                in< events::rollback >,
                in< events::execute >,
                in< events::exec_prepared >
            >;
        };

        using initial_state = starting;
        using transitions = transition_table<
            tr< starting,       events::ready_for_query,    idle            >,

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
    using transitions = transition_table<
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

using tran_idle_state = state<none, connection_fsm_def::transaction::idle>;

using connection_fsm = state_machine<connection_fsm_def, ::std::mutex>;

TEST(TranFSM, AllEvents)
{
    connection_fsm fsm;
    fsm.process_event(events::connect{});
    fsm.process_event(events::complete{});
    fsm.process_event(events::ready_for_query{});

    // Start transaction sequence
    fsm.process_event(events::begin{});
    fsm.process_event(events::ready_for_query{});

    // Simple query sequence
    fsm.process_event(events::execute{});
    fsm.process_event(events::row_description{});
    fsm.process_event(events::row_event{});
    fsm.process_event(events::row_event{});
    fsm.process_event(events::row_event{});
    fsm.process_event(events::row_event{});
    fsm.process_event(events::command_complete{});
    fsm.process_event(events::ready_for_query{});

    // Commit transaction sequence
    fsm.process_event(events::commit{});
    fsm.process_event(events::command_complete{});
    fsm.process_event(events::ready_for_query{});

    // Start transaction sequence
    fsm.process_event(events::begin{});
    fsm.process_event(events::ready_for_query{});

    // Extended query no data sequence
    fsm.process_event(events::exec_prepared{});

    // Commit transaction sequence
    fsm.process_event(events::commit{});
    fsm.process_event(events::command_complete{});
    fsm.process_event(events::ready_for_query{});

    // Terminate
    fsm.process_event(events::terminate{});
}

TEST(TranFSM, TranIdleState)
{
    none n;
    tran_idle_state state(n);
    EXPECT_EQ(actions::event_process_result::process_in_state,
            state.process_event(events::command_complete{}));
    EXPECT_EQ(actions::event_process_result::refuse,
            state.process_event(events::begin{}));
}

TEST(TranFSM, Transitions)
{
    ::std::mutex mutex;
    connection_fsm fsm;

    ::std::cerr << connection_fsm::initial_state_index
            << "/" << connection_fsm::inner_state_count << "\n";

    EXPECT_EQ(connection_fsm::initial_state_index, fsm.current_state());
}

}  /* namespace test */
}  /* namespace afsm */

