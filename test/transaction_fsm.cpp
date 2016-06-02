/*
 * transaction_sm.cpp
 *
 *  Created on: May 27, 2016
 *      Author: zmij
 */

#include <gtest/gtest.h>
#include <afsm/fsm.hpp>
#include <afsm/detail/debug_io.hpp>
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

struct transit_action {
    template < typename Event, typename FSM, typename SourceState, typename TargetState >
    void
    operator()(Event&&, FSM&, SourceState& src, TargetState& tgt) const
    {
        ::std::cerr << src.name() << " -> " << tgt.name() << "\n";
    }
};

struct state_name {
    virtual ~state_name() {}
    virtual ::std::string
    name() const = 0;
};

struct connection_fsm_def : def::state_machine<connection_fsm_def, state_name> {
    using connection_fsm = ::afsm::state_machine<connection_fsm_def, ::std::mutex>;

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
        template <typename Event, typename FSM>
        void
        on_exit(Event&&, FSM&)
        {
            ::std::cerr << "Exit closed\n";
        }

        ::std::string
        name() const override
        {
            return "closed";
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

        ::std::string
        name() const override
        {
            return "connecting";
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

        ::std::string
        name() const override
        {
            return "authorizing";
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

        ::std::string
        name() const override
        {
            return "idle";
        }
    };

    struct terminated : terminal_state<terminated> {
        template <typename Event, typename FSM>
        void
        on_enter(Event&&, FSM&)
        {
            ::std::cerr << "Enter terminated\n";
        }

        ::std::string
        name() const override
        {
            return "teminated";
        }
    };

    struct transaction : state_machine<transaction, state_name> {
        using transaction_fsm = ::afsm::inner_state_machine<connection_fsm, transaction>;

        transaction()
        {
            ::std::cerr << "Construct trasaction state\n";
        }
        template <typename Event, typename FSM>
        void
        on_enter(Event&&, FSM&)
        {
            ::std::cerr << "Enter transaction (" << name() << ")\n";
            ::std::cerr << "Initial state index " << transaction_fsm::initial_state_index
                    << " current state index " << fsm().current_state() << "\n";
        }
        template <typename Event, typename FSM>
        void
        on_exit(Event&&, FSM&)
        {
            ::std::cerr << "Exit transaction\n";
        }
        ::std::string
        name() const override
        {
            return "transaction " + fsm().current_state_base().name();
        }
        transaction_fsm&
        fsm()
        { return static_cast<transaction_fsm&>(*this); }
        transaction_fsm const&
        fsm() const
        { return static_cast<transaction_fsm const&>(*this); }

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
            ::std::string
            name() const override
            {
                return "starting";
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
            ::std::string
            name() const override
            {
                return "transaction idle";
            }
            using internal_transitions = transition_table<
                in< events::command_complete,   transit_action,   none >,
                in< events::ready_for_query,    transit_action,   none >
            >;
        };

        struct simple_query : state_machine<simple_query, state_name> {
            using simple_query_fsm = ::afsm::inner_state_machine<transaction_fsm, simple_query>;

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
            ::std::string
            name() const override
            {
                return "simple query " + fsm().current_state_base().name();
            }
            simple_query_fsm&
            fsm()
            { return static_cast<simple_query_fsm&>(*this); }
            simple_query_fsm const&
            fsm() const
            { return static_cast<simple_query_fsm const&>(*this); }

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
                ::std::string
                name() const override
                {
                    return "waiting results";
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
                ::std::string
                name() const override
                {
                    return "fetch data";
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

        struct extended_query : state_machine<extended_query, state_name> {
            using extended_query_fsm = ::afsm::inner_state_machine<transaction_fsm, extended_query>;
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
            ::std::string
            name() const override
            {
                return "extended query " + fsm().current_state_base().name();
            }
            extended_query_fsm&
            fsm()
            { return static_cast<extended_query_fsm&>(*this); }
            extended_query_fsm const&
            fsm() const
            { return static_cast<extended_query_fsm const&>(*this); }

            struct prepare  : state<prepare> {
                template <typename Event, typename FSM>
                void
                on_enter(Event&&, FSM&)
                {
                    ::std::cerr << "Enter prepare extended query\n";
                }
                template <typename Event, typename FSM>
                void
                on_exit(Event&&, FSM&)
                {
                    ::std::cerr << "Exit prepare extended query\n";
                }
                ::std::string
                name() const override
                {
                    return "prepare";
                }
            };
            struct parse    : state<parse> {
                template <typename Event, typename FSM>
                void
                on_enter(Event&&, FSM&)
                {
                    ::std::cerr << "Enter parse extended query\n";
                }
                template <typename Event, typename FSM>
                void
                on_exit(Event&&, FSM&)
                {
                    ::std::cerr << "Exit parse extended query\n";
                }
                ::std::string
                name() const override
                {
                    return "parse";
                }
                using internal_transitions = transition_table<
                    in< events::row_description >,
                    in< events::no_data >
                >;
            };
            struct bind     : state<bind> {
                template <typename Event, typename FSM>
                void
                on_enter(Event&&, FSM&)
                {
                    ::std::cerr << "Enter bind extended query\n";
                }
                template <typename Event, typename FSM>
                void
                on_exit(Event&&, FSM&)
                {
                    ::std::cerr << "Exit bind extended query\n";
                }
                ::std::string
                name() const override
                {
                    return "bind";
                }
            };
            struct exec     : state<exec> {
                template <typename Event, typename FSM>
                void
                on_enter(Event&&, FSM&)
                {
                    ::std::cerr << "Enter exec extended query\n";
                }
                template <typename Event, typename FSM>
                void
                on_exit(Event&&, FSM&)
                {
                    ::std::cerr << "Exit exec extended query\n";
                }
                ::std::string
                name() const override
                {
                    return "exec";
                }
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
            ::std::string
            name() const override
            {
                return "tran error";
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
            ::std::string
            name() const override
            {
                return "exiting";
            }
            using internal_transitions = transition_table<
                in< events::command_complete >,
                in< events::commit >,
                in< events::rollback >,
                in< events::execute >,
                in< events::exec_prepared >
            >;
        };

        using initial_state = starting;
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
    };

    using initial_state = closed;
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
};

using tran_idle_state = state<none, connection_fsm_def::transaction::idle>;

using connection_fsm = state_machine<connection_fsm_def, ::std::mutex>;

TEST(TranFSM, AllEvents)
{
    using actions::event_process_result;
    connection_fsm fsm;
    EXPECT_EQ(event_process_result::process, fsm.process_event(events::connect{}));
    EXPECT_EQ(event_process_result::process, fsm.process_event(events::complete{}));
    EXPECT_EQ(event_process_result::process, fsm.process_event(events::ready_for_query{}));

    // Start transaction sequence
    EXPECT_EQ(event_process_result::process, fsm.process_event(events::begin{}));
    EXPECT_EQ(event_process_result::process, fsm.process_event(events::ready_for_query{}));

    // Simple query sequence
    EXPECT_EQ(event_process_result::process, fsm.process_event(events::execute{}));
    EXPECT_EQ(event_process_result::process, fsm.process_event(events::row_description{}));
    EXPECT_EQ(event_process_result::process_in_state, fsm.process_event(events::row_event{}));
    EXPECT_EQ(event_process_result::process_in_state, fsm.process_event(events::row_event{}));
    EXPECT_EQ(event_process_result::process_in_state, fsm.process_event(events::row_event{}));
    EXPECT_EQ(event_process_result::process_in_state, fsm.process_event(events::row_event{}));
    EXPECT_EQ(event_process_result::process, fsm.process_event(events::command_complete{}));
    EXPECT_EQ(event_process_result::process, fsm.process_event(events::ready_for_query{}));

    // Commit transaction sequence
    EXPECT_EQ(event_process_result::process, fsm.process_event(events::commit{}));
    EXPECT_EQ(event_process_result::process_in_state, fsm.process_event(events::command_complete{}));
    EXPECT_EQ(event_process_result::process, fsm.process_event(events::ready_for_query{}));

    // Start transaction sequence
    EXPECT_EQ(event_process_result::process, fsm.process_event(events::begin{}));
    ::std::cerr << fsm.name() << "\n";
    EXPECT_EQ(event_process_result::process, fsm.process_event(events::ready_for_query{}));
    ::std::cerr << fsm.name() << "\n";

    // Extended query no data sequence
    EXPECT_EQ(event_process_result::process, fsm.process_event(events::exec_prepared{}));
    ::std::cerr << fsm.name() << "\n";

    // Commit transaction sequence
    EXPECT_EQ(event_process_result::process, fsm.process_event(events::commit{}));
    EXPECT_EQ(event_process_result::process, fsm.process_event(events::command_complete{}));
    EXPECT_EQ(event_process_result::process, fsm.process_event(events::ready_for_query{}));

    // Terminate
    EXPECT_EQ(event_process_result::process, fsm.process_event(events::terminate{}));
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

