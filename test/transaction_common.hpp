/*
 * transaction_events.hpp
 *
 *  Created on: 28 нояб. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef TEST_TRANSACTION_COMMON_HPP_
#define TEST_TRANSACTION_COMMON_HPP_

#include <string>
#include <afsm/detail/debug_io.hpp>
#include <pushkin/ansi_colors.hpp>
#include <pushkin/util/demangle.hpp>
#include <iostream>
#include <iomanip>

namespace afsm {
namespace test {

namespace events {

struct connect {
    static ::std::string const name;
};
struct complete {
    static ::std::string const name;
};
struct ready_for_query {
    static ::std::string const name;
};
struct begin {
    static ::std::string const name;
};
struct terminate {
    static ::std::string const name;
};

struct execute {
    static ::std::string const name;
};
struct exec_prepared {
    static ::std::string const name;
};

struct commit {
    static ::std::string const name;
};
struct rollback {
    static ::std::string const name;
};

struct conn_error {
    static ::std::string const name;
};
struct query_error {
    static ::std::string const name;
};
struct client_error {
    static ::std::string const name;
};

struct row_description {
    static ::std::string const name;
};
struct no_data {
    static ::std::string const name;
};
struct row_event {
    static ::std::string const name;
};
struct command_complete {
    static ::std::string const name;
};

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

struct connection_observer : ::afsm::detail::null_observer {
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

    template < typename FSM, typename State >
    void
    state_cleared(FSM const&, State const&) const noexcept
    {
        using ::psst::ansi_color;
        using ::psst::util::demangle;
        ::std::cerr
             << (ansi_color::red | ansi_color::bright)
             << ::std::setw(event_name_width) << ::std::setfill('*')
             << "*" << ansi_color::clear  << ::std::setfill(' ')
             << ": State cleared " << demangle< typename State::state_definition_type >() <<"\n";
    }
    template < typename FSM, typename SourceState, typename TargetState, typename Event >
    void
    state_changed(FSM const&, SourceState const&, TargetState const&, Event const&) const noexcept
    {
        using ::psst::ansi_color;
        using ::psst::util::demangle;
        ::std::cerr
             << (ansi_color::blue | ansi_color::bright)
             << ::std::setw(event_name_width) << ::std::setfill('*')
             << "*" << ansi_color::clear  << ::std::setfill(' ')
             << ": State changed " << demangle< typename SourceState::state_definition_type >()
             << " -> " << demangle< typename TargetState::state_definition_type >()
             << " (" << demangle<Event>() << ")\n";
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

}  /* namespace test */
}  /* namespace afsm */


#endif /* TEST_TRANSACTION_COMMON_HPP_ */
