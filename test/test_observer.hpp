/*
 * test_observer.hpp
 *
 *  Created on: Dec 21, 2016
 *      Author: zmij
 */

#ifndef TEST_OBSERVER_HPP_
#define TEST_OBSERVER_HPP_

#include <afsm/fsm.hpp>
#include <afsm/detail/debug_io.hpp>
#include <pushkin/ansi_colors.hpp>
#include <pushkin/util/demangle.hpp>
#include <string>
#include <iostream>
#include <iomanip>

namespace afsm {
namespace test {

namespace {

int const event_name_width = 17;

}  /* namespace  */

inline ::std::vector< ::std::string >
split_qualified_name(::std::string const& name)
{
    ::std::vector< ::std::string > names{::std::string{}};
    for (auto c = name.begin(); c != name.end(); ++c) {
        if (*c == ':') {
            if (!names.back().empty())
                names.push_back(::std::string{});
        } else {
            names.back().push_back(*c);
        }
    }
    return names;
}

inline ::std::string
get_name_components(::std::string const& name, int count)
{
    auto names = split_qualified_name(name);
    ::std::ostringstream os;
    int offset = names.size() - count;
    if (offset < 0)
        offset = 0;
    auto b = names.begin() + offset;
    for (auto p = b; p != names.end(); ++p) {
        if (p != b) {
            os << "::";
        }
        os << *p;
    }
    return os.str();
}

inline ::std::string
get_name_components_after(::std::string const& name, ::std::string const& start)
{
    if (start.empty()) {
        return name;
    } else {
        auto names = split_qualified_name(name);
        ::std::ostringstream os;
        auto p = names.begin();
        while (p != names.end() && *p != start) ++p;
        if (p != names.end()) {
            ++p;
        } else {
            p = names.begin();
        }
        auto b = p;
        for (; p != names.end(); ++p) {
            if (p != b)
                os << "::";
            os << *p;
        }

        return os.str();
    }
}

struct test_fsm_observer : ::afsm::detail::null_observer {
    test_fsm_observer()
        : def_name{}{}
    test_fsm_observer(::std::string const& n)
        : def_name{n}{}

    template < typename FSM, typename Event >
    void
    start_process_event(FSM const&, Event const&) const noexcept
    {
        using ::psst::ansi_color;
        using ::psst::util::demangle;
        ::std::cerr
             << (ansi_color::green | ansi_color::dim)
             << ::std::setw(event_name_width) << ::std::left
             << get_name_components(demangle<Event>(), 1)
             << ansi_color::clear
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
             << ": State cleared "
             << get_name_components_after(
                 demangle< typename State::state_definition_type >(),
                 def_name) <<"\n";
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
             << ": State changed "
             << get_name_components_after(
                     demangle< typename SourceState::state_definition_type >(),
                     def_name)
             << " -> "
             << get_name_components_after(
                     demangle< typename TargetState::state_definition_type >(),
                     def_name)
             << " (" << get_name_components(demangle<Event>(), 1) << ")\n";
    }

    template < typename FSM, typename Event >
    void
    processed_in_state(FSM const&, Event const&) const noexcept
    {
        using ::psst::ansi_color;
        using ::psst::util::demangle;
        ::std::cerr
             << (ansi_color::blue | ansi_color::dim)
             << ::std::setw(event_name_width) << ::std::left
             << get_name_components(demangle<Event>(), 1)
             << ansi_color::clear
             << ": Processed in state\n";
    }

    template < typename FSM, typename Event >
    void
    enqueue_event(FSM const&, Event const&) const noexcept
    {
        using ::psst::ansi_color;
        using ::psst::util::demangle;
        ::std::cerr
             << (ansi_color::blue | ansi_color::dim)
             << ::std::setw(event_name_width) << ::std::left
             << get_name_components(demangle<Event>(), 1)
             << ansi_color::clear
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
        using ::psst::ansi_color;
        using ::psst::util::demangle;
        ::std::cerr
             << (ansi_color::red | ansi_color::dim)
             << ::std::setw(event_name_width) << ::std::left
             << get_name_components(demangle<Event>(), 1)
             << ansi_color::clear
             << ": Defer event\n";
    }

    template < typename FSM >
    void
    start_process_deferred_queue(FSM const&, ::std::size_t size) const noexcept
    {
        using ::psst::ansi_color;
        ::std::cerr
             << (ansi_color::blue | ansi_color::bright)
             << ::std::setw(event_name_width) << ::std::setfill('*')
             << "*" << ansi_color::clear  << ::std::setfill(' ')
             << ": Start processing deferred event queue size " << size << "\n";
    }
    template < typename FSM >
    void
    end_process_deferred_queue(FSM const&, ::std::size_t size) const noexcept
    {
        using ::psst::ansi_color;
        ::std::cerr
             << (ansi_color::blue | ansi_color::bright)
             << ::std::setw(event_name_width) << ::std::setfill('*')
             << "*" << ansi_color::clear  << ::std::setfill(' ')
             << ": End processing deferred event queue remain " << size << "\n";
    }
    template < typename FSM >
    void
    skip_processing_deferred_queue(FSM const& fsm) const noexcept
    {
        using ::psst::ansi_color;
        auto const& handled = fsm.current_handled_events();
        auto const& deferred = fsm.current_deferred_events();
        ::std::cerr
             << (ansi_color::yellow | ansi_color::bright)
             << ::std::setw(event_name_width) << ::std::setfill('*') << "*"
             << ansi_color::clear  << ::std::setfill(' ')
             << ": Skip processing deferred event queue. FSM can handle "
             << handled.size() << " event types now. Deferred "
             << deferred.size() << " event types.\n"
         ;
    }
    template < typename FSM >
    void
    postpone_deferred_events(FSM const& fsm, ::std::size_t count) const noexcept
    {
        using ::psst::ansi_color;
        ::std::cerr
             << (ansi_color::yellow)
             << ::std::setw(event_name_width) << ::std::setfill('*') << "*"
             << ansi_color::clear  << ::std::setfill(' ')
             << ": Postpone " << count << " deferred events\n";
    }
    template < typename FSM >
    void
    drop_deferred_event(FSM const& fsm) const noexcept
    {
        using ::psst::ansi_color;
        ::std::cerr
             << (ansi_color::red | ansi_color::bright)
             << ::std::setw(event_name_width) << ::std::setfill('*') << "*"
             << ansi_color::clear  << ::std::setfill(' ')
             << ": Drop " << count << " deferred event\n";
    };

    template < typename FSM, typename Event >
    void
    reject_event(FSM const&, Event const&) const noexcept
    {
        using ::psst::ansi_color;
        using ::psst::util::demangle;
        ::std::cerr
             << (ansi_color::red | ansi_color::bright)
             << ::std::setw(event_name_width) << ::std::left
             << get_name_components(demangle<Event>(), 1)
             << ansi_color::clear
             << ": Reject event.\n";
    }

    ::std::string def_name;
};

}  /* namespace test */
}  /* namespace afsm */

#endif /* TEST_OBSERVER_HPP_ */
