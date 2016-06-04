/*
 * transaction_sm.cpp
 *
 *  Created on: May 27, 2016
 *      Author: zmij
 */

#include <gtest/gtest.h>
#include <afsm/fsm.hpp>
#include <afsm/detail/debug_io.hpp>
#include <pushkin/ansi_colors.hpp>
#include <iostream>
#include <iomanip>

namespace afsm {
namespace test {

namespace events {

struct connect {
    static ::std::string const name;
};
::std::string const connect::name{"connect"};
struct complete {
    static ::std::string const name;
};
::std::string const complete::name{"complete"};
struct ready_for_query {
    static ::std::string const name;
};
::std::string const ready_for_query::name{"ready_for_query"};
struct begin {
    static ::std::string const name;
};
::std::string const begin::name{"begin"};
struct terminate {
    static ::std::string const name;
};
::std::string const terminate::name{"terminate"};

struct execute {
    static ::std::string const name;
};
::std::string const execute::name{"execute"};
struct exec_prepared {
    static ::std::string const name;
};
::std::string const exec_prepared::name{"exec_prepared"};

struct commit {
    static ::std::string const name;
};
::std::string const commit::name{"commit"};
struct rollback {
    static ::std::string const name;
};
::std::string const rollback::name{"rollback"};

struct conn_error {
    static ::std::string const name;
};
::std::string const conn_error::name{"conn_error"};
struct query_error {
    static ::std::string const name;
};
::std::string const query_error::name{"query_error"};
struct client_error {
    static ::std::string const name;
};
::std::string const client_error::name{"client_error"};

struct row_description {
    static ::std::string const name;
};
::std::string const row_description::name{"row_description"};
struct no_data {
    static ::std::string const name;
};
::std::string const no_data::name{"no_data"};
struct row_event {
    static ::std::string const name;
};
::std::string const row_event::name{"row_event"};
struct command_complete {
    static ::std::string const name;
};
::std::string const command_complete::name{"command_complete"};

}  /* namespace events */

namespace {

int const event_name_width = 17;

}  /* namespace  */

struct transit_action {
    template < typename Event, typename FSM, typename SourceState, typename TargetState >
    void
    operator()(Event&&, FSM&, SourceState& src, TargetState& tgt) const
    {
        using ::psst::ansi_color;
        ::std::cerr
                << (ansi_color::cyan | ansi_color::bright)
                << ::std::setw(event_name_width) << ::std::left
                << Event::name << ansi_color::clear
                << ": " << src.name() << " -> " << tgt.name() << "\n";
    }
    template < typename FSM, typename SourceState, typename TargetState >
    void
    operator()(none&&, FSM&, SourceState& src, TargetState& tgt) const
    {
        using ::psst::ansi_color;
        ::std::cerr
                << (ansi_color::red | ansi_color::bright)
                << ::std::setw(event_name_width) << ::std::left
                << "[default]" << ansi_color::clear
                << ": " << src.name() << " -> " << tgt.name() << "\n";
    }
};

struct state_name {
    virtual ~state_name() {}
    virtual ::std::string
    name() const = 0;

    template < typename Event, typename FSM >
    void
    on_enter(Event&&, FSM&)
    {
        using decayed_event = typename ::std::decay<Event>::type;
        using ::psst::ansi_color;
        ::std::cerr
             << (ansi_color::blue | ansi_color::bright)
             << ::std::setw(event_name_width) << ::std::left
             << decayed_event::name << ansi_color::clear
             << ": Enter " << name() << "\n";
    }
    template < typename FSM >
    void
    on_enter(none&&, FSM&)
    {
        using ::psst::ansi_color;
        ::std::cerr
                << ansi_color::red
                << ::std::setw(event_name_width) << ::std::left
                << "[default]" << ansi_color::clear
                << ": Enter " << name() << "\n";
    }
    template < typename Event, typename FSM >
    void
    on_exit(Event&&, FSM&)
    {
        using decayed_event = typename ::std::decay<Event>::type;
        using ::psst::ansi_color;
        ::std::cerr
             << (ansi_color::cyan | ansi_color::dim)
             << ::std::setw(event_name_width) << ::std::left
             << decayed_event::name << ansi_color::clear
             << ": Exit " << name() << "\n";
    }
    template < typename FSM >
    void
    on_exit(none const&, FSM&)
    {
        using ::psst::ansi_color;
        ::std::cerr
                << (ansi_color::red | ansi_color::dim)
                << ::std::setw(event_name_width) << ::std::left
                << "[default]" << ansi_color::clear
                << ": Exit " << name() << "\n";
    }
};

struct connection_observer {
    template < typename FSM, typename Event >
    void
    start_process_event(FSM const&, Event const&) const noexcept
    {
        using decayed_event = typename ::std::decay<Event>::type;
        using ::psst::ansi_color;
        ::std::cerr
             << (ansi_color::green | ansi_color::dim)
             << ::std::setw(event_name_width) << ::std::left
             << decayed_event::name << ansi_color::clear
             << ": Start processing\n";
    }

    template < typename FSM >
    void
    start_process_event(FSM const&, none const&) const noexcept
    {
        using ::psst::ansi_color;
        ::std::cerr
             << (ansi_color::green | ansi_color::dim)
             << ::std::setw(event_name_width) << ::std::left
             << "[default]" << ansi_color::clear
             << ": Start processing\n";
    }

    template < typename FSM >
    void
    state_changed(FSM const&) const noexcept
    {
        using ::psst::ansi_color;
        ::std::cerr
             << (ansi_color::blue | ansi_color::bright)
             << ::std::setw(event_name_width) << ::std::setfill('*')
             << "*" << ansi_color::clear  << ::std::setfill(' ')
             << ": State changed\n";
    }

    template < typename FSM, typename Event >
    void
    processed_in_state(FSM const&, Event const&) const noexcept
    {
        using decayed_event = typename ::std::decay<Event>::type;
        using ::psst::ansi_color;
        ::std::cerr
             << (ansi_color::blue | ansi_color::dim)
             << ::std::setw(event_name_width) << ::std::left
             << decayed_event::name << ansi_color::clear
             << ": Processed in state\n";
    }

    template < typename FSM, typename Event >
    void
    enqueue_event(FSM const&, Event const&) const noexcept
    {
        using decayed_event = typename ::std::decay<Event>::type;
        using ::psst::ansi_color;
        ::std::cerr
             << (ansi_color::blue | ansi_color::dim)
             << ::std::setw(event_name_width) << ::std::left
             << decayed_event::name << ansi_color::clear
             << ": Enqueue event\n";
    }

    template < typename FSM >
    void
    start_process_events_queue(FSM const&) const noexcept
    {
        using ::psst::ansi_color;
        ::std::cerr
             << (ansi_color::blue | ansi_color::bright)
             << ::std::setw(event_name_width) << ::std::setfill('*')
             << "*" << ansi_color::clear  << ::std::setfill(' ')
             << ": Start processing event queue\n";
    }
    template < typename FSM >
    void
    end_process_events_queue(FSM const&) const noexcept
    {
        using ::psst::ansi_color;
        ::std::cerr
             << (ansi_color::blue | ansi_color::bright)
             << ::std::setw(event_name_width) << ::std::setfill('*')
             << "*" << ansi_color::clear  << ::std::setfill(' ')
             << ": End processing event queue\n";
    }

    template < typename FSM, typename Event >
    void
    defer_event(FSM const&, Event const&) const noexcept
    {
        using decayed_event = typename ::std::decay<Event>::type;
        using ::psst::ansi_color;
        ::std::cerr
             << (ansi_color::red | ansi_color::dim)
             << ::std::setw(event_name_width) << ::std::left
             << decayed_event::name << ansi_color::clear
             << ": Defer event\n";
    }

    template < typename FSM >
    void
    start_process_deferred_queue(FSM const&) const noexcept
    {
        using ::psst::ansi_color;
        ::std::cerr
             << (ansi_color::blue | ansi_color::bright)
             << ::std::setw(event_name_width) << ::std::setfill('*')
             << "*" << ansi_color::clear  << ::std::setfill(' ')
             << ": Start processing deferred event queue\n";
    }
    template < typename FSM >
    void
    end_process_deferred_queue(FSM const&) const noexcept
    {
        using ::psst::ansi_color;
        ::std::cerr
             << (ansi_color::blue | ansi_color::bright)
             << ::std::setw(event_name_width) << ::std::setfill('*')
             << "*" << ansi_color::clear  << ::std::setfill(' ')
             << ": End processing deferred event queue\n";
    }

    template < typename FSM, typename Event >
    void
    reject_event(FSM const& fsm, Event const&) const noexcept
    {
        using decayed_event = typename ::std::decay<Event>::type;
        using ::psst::ansi_color;
        ::std::cerr
             << (ansi_color::red | ansi_color::bright)
             << ::std::setw(event_name_width) << ::std::left
             << decayed_event::name << ansi_color::clear
             << ": Reject event. State " << fsm.name() << "\n";
    }
};

struct connection_fsm_def : def::state_machine<connection_fsm_def, state_name> {
    using connection_fsm =
            ::afsm::state_machine<
                 connection_fsm_def, ::std::mutex, connection_observer>;

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

    struct transaction : state_machine<transaction, state_name> {
        using transaction_fsm = ::afsm::inner_state_machine<transaction, connection_fsm>;

        transaction()
        {
            ::std::cerr << "Construct trasaction state\n";
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
            using deferred_events = type_tuple<
                events::execute,
                events::exec_prepared,
                events::commit,
                events::rollback
            >;
            ::std::string
            name() const override
            {
                return "starting";
            }
            using internal_transitions = transition_table<
                in< events::command_complete,   transit_action,   none >
            >;
        };

        struct idle : state<idle> {
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
            using simple_query_fsm = ::afsm::inner_state_machine<simple_query, transaction_fsm>;
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
                ::std::string
                name() const override
                {
                    return "waiting results";
                }
                using internal_transitions = transition_table<
                    in< events::command_complete,   transit_action >
                >;
            };
            struct fetch_data : state<fetch_data>{
                ::std::string
                name() const override
                {
                    return "fetch data";
                }
                using internal_transitions = transition_table<
                    in< events::row_event, transit_action >
                >;
            };

            using initial_state = waiting;
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
        };

        struct extended_query : state_machine<extended_query, state_name> {
            using extended_query_fsm = ::afsm::inner_state_machine<extended_query, transaction_fsm>;
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
                ::std::string
                name() const override
                {
                    return "prepare";
                }
            };
            struct parse    : state<parse> {
                ::std::string
                name() const override
                {
                    return "parse";
                }
                using internal_transitions = transition_table<
                    in< events::row_description,    transit_action >,
                    in< events::no_data,            transit_action >
                >;
            };
            struct bind     : state<bind> {
                ::std::string
                name() const override
                {
                    return "bind";
                }
            };
            struct exec     : state<exec> {
                ::std::string
                name() const override
                {
                    return "exec";
                }
                using internal_transitions = transition_table<
                    in< events::row_event,          transit_action  >,
                    in< events::command_complete,   transit_action  >
                >;
            };

            using initial_state = prepare;
            using transitions = transition_table<
                tr< prepare,    none,                       parse,      transit_action >,
                tr< parse,      events::ready_for_query,    bind,       transit_action >,
                tr< bind,       events::ready_for_query,    exec,       transit_action >
            >;
        };

        struct tran_error : state<tran_error> {
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
            ::std::string
            name() const override
            {
                return "exiting";
            }
            using internal_transitions = transition_table<
                in< events::command_complete,   transit_action >,
                in< events::commit,             transit_action >,
                in< events::rollback,           transit_action >,
                in< events::execute,            transit_action >,
                in< events::exec_prepared,      transit_action >
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

using connection_fsm = state_machine<connection_fsm_def, ::std::mutex, connection_observer>;

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
}

TEST(TranFSM, AllEvents)
{
    using actions::event_process_result;
    using ::psst::ansi_color;
    connection_fsm fsm;
    fsm.make_observer();

    EXPECT_EQ(event_process_result::process, fsm.process_event(events::connect{}));
    EXPECT_EQ(event_process_result::process, fsm.process_event(events::complete{}));
    EXPECT_EQ(event_process_result::process, fsm.process_event(events::ready_for_query{}));

    begin_transaction(fsm);

    // Simple query sequence
    EXPECT_EQ(event_process_result::process, fsm.process_event(events::execute{}));
    EXPECT_EQ(event_process_result::process, fsm.process_event(events::row_description{}));
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

TEST(TranFSM, RealEventSequence)
{
    using actions::event_process_result;
    using ::psst::ansi_color;
    connection_fsm fsm;
    fsm.make_observer();

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

