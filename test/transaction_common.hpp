/*
 * transaction_events.hpp
 *
 *  Created on: 28 нояб. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef TEST_TRANSACTION_COMMON_HPP_
#define TEST_TRANSACTION_COMMON_HPP_

#include "test_observer.hpp"
#include <afsm/detail/debug_io.hpp>
#include <pushkin/ansi_colors.hpp>
#include <pushkin/util/demangle.hpp>

#include <iomanip>
#include <iostream>
#include <string>

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

} /* namespace events */

struct transit_action {
    template <typename Event, typename FSM, typename SourceState, typename TargetState>
    void
    operator()(Event&&, FSM&, SourceState& src, TargetState& tgt) const
    {
        using ::psst::ansi_color;
        ::std::cerr << (ansi_color::cyan | ansi_color::bright) << ::std::setw(event_name_width)
                    << ::std::left << Event::name << ansi_color::clear << ": " << src.name()
                    << " -> " << tgt.name() << "\n";
    }
    template <typename FSM, typename SourceState, typename TargetState>
    void
    operator()(none&&, FSM&, SourceState& src, TargetState& tgt) const
    {
        using ::psst::ansi_color;
        ::std::cerr << (ansi_color::red | ansi_color::bright) << ::std::setw(event_name_width)
                    << ::std::left << "[default]" << ansi_color::clear << ": " << src.name()
                    << " -> " << tgt.name() << "\n";
    }
};

struct state_name {
    virtual ~state_name() {}
    virtual ::std::string
    name() const = 0;

    template <typename Event, typename FSM>
    void
    on_enter(Event&&, FSM&)
    {
        using decayed_event = typename ::std::decay<Event>::type;
        using ::psst::ansi_color;
        ::std::cerr << (ansi_color::blue | ansi_color::bright) << ::std::setw(event_name_width)
                    << ::std::left << decayed_event::name << ansi_color::clear << ": Enter "
                    << name() << "\n";
    }
    template <typename FSM>
    void
    on_enter(none&&, FSM&)
    {
        using ::psst::ansi_color;
        ::std::cerr << ansi_color::red << ::std::setw(event_name_width) << ::std::left
                    << "[default]" << ansi_color::clear << ": Enter " << name() << "\n";
    }
    template <typename Event, typename FSM>
    void
    on_exit(Event&&, FSM&)
    {
        using decayed_event = typename ::std::decay<Event>::type;
        using ::psst::ansi_color;
        ::std::cerr << (ansi_color::cyan | ansi_color::dim) << ::std::setw(event_name_width)
                    << ::std::left << decayed_event::name << ansi_color::clear << ": Exit "
                    << name() << "\n";
    }
    template <typename FSM>
    void
    on_exit(none const&, FSM&)
    {
        using ::psst::ansi_color;
        ::std::cerr << (ansi_color::red | ansi_color::dim) << ::std::setw(event_name_width)
                    << ::std::left << "[default]" << ansi_color::clear << ": Exit " << name()
                    << "\n";
    }
};

} /* namespace test */
} /* namespace afsm */

#endif /* TEST_TRANSACTION_COMMON_HPP_ */
