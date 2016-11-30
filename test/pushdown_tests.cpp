/*
 * pushdown_tests.cpp
 *
 *  Created on: Nov 29, 2016
 *      Author: zmij
 */

#include <gtest/gtest.h>

#include <afsm/fsm.hpp>
#include <pushkin/ansi_colors.hpp>
#include <pushkin/util/demangle.hpp>

#include <iostream>

namespace afsm {
namespace test {

namespace events {

struct bool_literal {};
struct integer_literal {};
struct float_literal {};
struct string_literal {};
struct null_literal {};

struct start_array {};
struct end_array {};

struct start_object {};
struct end_object {};

struct comma{};
struct colon{};

}  /* namespace events */

enum class value_context {
    none,
    array,
    array_first,
    object
};

struct json_parser_def : def::state_machine< json_parser_def > {
    struct context : state_machine< context > {
        //@{
        /** @name Substates */
        struct start : state<start> {
            template < typename Event, typename FSM >
            void
            on_enter(Event&&, FSM&)
            {
                using ::psst::ansi_color;
                using ::psst::util::demangle;
                ::std::cerr << (ansi_color::cyan | ansi_color::bright)
                        << "start: " << ansi_color::clear
                        << demangle<Event>() << "\n";
            }

            template < typename FSM >
            void
            on_enter(events::start_array&&, FSM& fsm)
            {
                using ::psst::ansi_color;
                using ::psst::util::demangle;
                ::std::cerr << (ansi_color::cyan | ansi_color::bright)
                        << "start: " << ansi_color::clear
                        << "array context (first element)\n";

                fsm.context_ = value_context::array_first;
            }
            template < typename FSM >
            void
            on_enter(events::comma&&, FSM& fsm)
            {
                using ::psst::ansi_color;
                using ::psst::util::demangle;
                ::std::cerr << (ansi_color::cyan | ansi_color::bright)
                        << "start: " << ansi_color::clear
                        << "array context\n";
                fsm.context_ = value_context::array;
            }
            template < typename FSM >
            void
            on_enter(events::colon&&, FSM& fsm)
            {
                using ::psst::ansi_color;
                using ::psst::util::demangle;
                ::std::cerr << (ansi_color::cyan | ansi_color::bright)
                        << "start: " << ansi_color::clear
                        << "object context\n";
                fsm.context_ = value_context::object;
            }
        };

        struct array : state_machine< array > {
            struct value : push< value, json_parser_def > {};

            using initial_state = value;

            using transitions = transition_table<
                tr< value,  events::comma,          value >
            >;
        };

        struct object : state_machine< object > {
            struct name : state<name> {
                template < typename FSM >
                void
                on_exit(events::string_literal const&, FSM& fsm)
                {
                    ++fsm.size;
                }
            };
            struct colon : state< colon > {};
            struct value : push< value, json_parser_def > {};

            using initial_state = name;

            using transitions = transition_table<
                tr< name,   events::string_literal, colon   >,
                tr< colon,  events::colon,          value   >,
                tr< value,  events::comma,          name    >
            >;
            ::std::size_t   size     = 0;
        };

        struct end : pop< end, json_parser_def > {};
        //@}
        //@{
        /** @name Guards */
        template < value_context Ctx >
        struct in_context {
            template < typename FSM, typename State >
            bool
            operator()(FSM const& fsm, State const&) const
            {
                return fsm.context_ == Ctx;
            }
        };
        using in_first_element = in_context<value_context::array_first>;
        struct can_close_object {
            template < typename FSM, typename State >
            bool
            operator()(FSM const& fsm, State const& state) const
            {
                return (state.size == 0 && fsm.template is_in_state< object::name >())
                        || fsm.template is_in_state< object::value >();
            }
        };
        //@}
        //@{
        /** @name Actions */
        struct dup_event {
            template < typename Event, typename FSM, typename SourceState, typename TargetState >
            void
            operator()(Event&& evt, FSM& fsm, SourceState&, TargetState&)
            {
                root_machine(fsm).process_event(::std::forward<Event>(evt));
            }
        };
        //@}

        using initial_state = start;
        using transitions = transition_table<
            /*  Start | Event                     | Next  | Action    | Guard               */
            /*--------+---------------------------+-------+-----------+---------------------*/
            tr< start , events::bool_literal      , end   , none      , none                >,
            tr< start , events::integer_literal   , end   , none      , none                >,
            tr< start , events::float_literal     , end   , none      , none                >,
            tr< start , events::string_literal    , end   , none      , none                >,
            tr< start , events::null_literal      , end   , none      , none                >,

            /*--------+---------------------------+-------+-----------+---------------------*/
            tr< start , events::start_array       , array , none      , none                >,
            tr< array , events::end_array         , end   , none      , none                >,
            tr< start , events::end_array         , end   , dup_event , in_first_element    >,

            /*--------+---------------------------+-------+-----------+---------------------*/
            tr< start , events::start_object      , object, none      , none                >,
            tr< object, events::end_object        , end   , none      , can_close_object    >
        >;

        value_context   context_ = value_context::none;
    };

    using orthogonal_regions = type_tuple<context>;
};

using json_parser_fsm = state_machine<json_parser_def>;

static_assert(def::traits::is_state<json_parser_def::context::array::value>::value, "");
static_assert(def::traits::is_pushdown<json_parser_def::context::array::value>::value, "");
static_assert(def::traits::is_pushdown<json_parser_def::context::object::value>::value, "");
static_assert(def::traits::is_popup<json_parser_def::context::end>::value, "");

static_assert(def::traits::pushes<json_parser_def::context::array::value, json_parser_def>::value, "");
static_assert(def::traits::pushes<json_parser_def::context::object::value, json_parser_def>::value, "");

static_assert(def::state_pushes<json_parser_def::context::array::value, json_parser_def>::value, "");
static_assert(def::state_pushes<json_parser_def::context::object::value, json_parser_def>::value, "");

static_assert(def::traits::pops<json_parser_def::context::end, json_parser_def>::value, "");
static_assert(def::state_pops<json_parser_def::context::end, json_parser_def>::value, "");

static_assert(def::contains_pushdowns<json_parser_def::context::array>::value, "");
static_assert(def::contains_pushdowns<json_parser_def::context::object>::value, "");
static_assert(!def::contains_popups<json_parser_def::context::array>::value, "");
static_assert(!def::contains_popups<json_parser_def::context::object>::value, "");

static_assert(def::contains_pushdowns<json_parser_def>::value, "");
static_assert(def::contains_popups<json_parser_def>::value, "");

static_assert(def::is_pushed<json_parser_def>::value, "");
static_assert(!def::is_pushed<json_parser_def::context::array>::value, "");
static_assert(!def::is_pushed<json_parser_def::context::object>::value, "");

static_assert(def::is_popped<json_parser_def>::value, "");
static_assert(!def::is_popped<json_parser_def::context::array>::value, "");
static_assert(!def::is_popped<json_parser_def::context::object>::value, "");

static_assert(def::has_pushdown_stack<json_parser_def>::value, "");
static_assert(!def::has_pushdown_stack<json_parser_def::context::array>::value, "");
static_assert(!def::has_pushdown_stack<json_parser_def::context::object>::value, "");

static_assert(::std::is_same<
        def::state_path<json_parser_def, json_parser_def>::type,
        ::psst::meta::type_tuple<json_parser_def>
    >::value, "");
static_assert(::std::is_same<
        def::state_path<json_parser_def::context::start, json_parser_def::context::start>::type,
        ::psst::meta::type_tuple<json_parser_def::context::start>
    >::value, "");
static_assert(::std::is_same<
        def::state_path<json_parser_def, json_parser_def::context::start>::type,
        ::psst::meta::type_tuple<
             json_parser_def,
             json_parser_def::context,
             json_parser_def::context::start>
    >::value, "");

static_assert(::std::is_same<
        def::state_path<json_parser_def, json_parser_def::context::object::name>::type,
        ::psst::meta::type_tuple<
             json_parser_def,
             json_parser_def::context,
             json_parser_def::context::object,
             json_parser_def::context::object::name
        >
    >::value, "");

static_assert(::std::is_same<
        def::state_path<json_parser_fsm, json_parser_def::context::object::name>::type,
        ::psst::meta::type_tuple<
             json_parser_def,
             json_parser_def::context,
             json_parser_def::context::object,
             json_parser_def::context::object::name
        >
    >::value, "");

TEST(Pushdown, ToTheEnd)
{
    json_parser_fsm fsm;

    EXPECT_TRUE(fsm.is_in_state<json_parser_def>());
    EXPECT_TRUE(fsm.is_in_state<json_parser_fsm::context::start>());
    EXPECT_TRUE(done(fsm.process_event(events::bool_literal{})));
    EXPECT_TRUE(fsm.is_in_state<json_parser_fsm::context::end>());
}

TEST(Pushdown, PushPopState)
{
    json_parser_fsm fsm;

    EXPECT_TRUE(fsm.is_in_state<json_parser_def>());
    EXPECT_TRUE(fsm.is_in_state<json_parser_fsm::context::start>());
    EXPECT_TRUE(done(fsm.process_event(events::start_array{})));
    EXPECT_TRUE(fsm.is_in_state<json_parser_fsm::context::start>());
    EXPECT_EQ(2, fsm.stack_size());
    EXPECT_TRUE(done(fsm.process_event(events::null_literal{})));
    EXPECT_EQ(1, fsm.stack_size());
    EXPECT_TRUE(fsm.is_in_state<json_parser_fsm::context::array>());
    EXPECT_TRUE(done(fsm.process_event(events::end_array{})));
    EXPECT_TRUE(fsm.is_in_state<json_parser_fsm::context::end>());
}

TEST(Pushdown, PushTwo)
{
    json_parser_fsm fsm;

    EXPECT_TRUE(fsm.is_in_state<json_parser_def>());
    EXPECT_TRUE(fsm.is_in_state<json_parser_fsm::context::start>());
    EXPECT_TRUE(done(fsm.process_event(events::start_array{})));
    EXPECT_TRUE(fsm.is_in_state<json_parser_fsm::context::start>());
    EXPECT_EQ(2, fsm.stack_size());

    EXPECT_TRUE(done(fsm.process_event(events::start_array{})));
    EXPECT_TRUE(fsm.is_in_state<json_parser_fsm::context::start>());
    EXPECT_EQ(3, fsm.stack_size());
    EXPECT_TRUE(done(fsm.process_event(events::null_literal{})));
    EXPECT_EQ(2, fsm.stack_size());
    EXPECT_TRUE(fsm.is_in_state<json_parser_fsm::context::array>());

    EXPECT_TRUE(done(fsm.process_event(events::comma{})));


    EXPECT_TRUE(fsm.is_in_state<json_parser_fsm::context::start>());
    EXPECT_EQ(3, fsm.stack_size());

    EXPECT_FALSE(ok(fsm.process_event(events::end_array{})))
            << "Cannot close array immediately after comma";

    EXPECT_TRUE(done(fsm.process_event(events::null_literal{})));
    EXPECT_EQ(2, fsm.stack_size());
    EXPECT_TRUE(fsm.is_in_state<json_parser_fsm::context::array>());

    EXPECT_TRUE(done(fsm.process_event(events::end_array{})));

    EXPECT_EQ(1, fsm.stack_size());
    EXPECT_TRUE(fsm.is_in_state<json_parser_fsm::context::array>());
    EXPECT_TRUE(done(fsm.process_event(events::end_array{})));
    EXPECT_TRUE(fsm.is_in_state<json_parser_fsm::context::end>());
}

TEST(Pushdown, EmptyArray)
{
    json_parser_fsm fsm;

    EXPECT_TRUE(fsm.is_in_state<json_parser_def>());
    EXPECT_TRUE(fsm.is_in_state<json_parser_fsm::context::start>());
    EXPECT_TRUE(done(fsm.process_event(events::start_array{})));
    EXPECT_TRUE(fsm.is_in_state<json_parser_fsm::context::start>());
    EXPECT_EQ(2, fsm.stack_size());

    EXPECT_TRUE(done(fsm.process_event(events::end_array{})));
    EXPECT_EQ(1, fsm.stack_size());
    EXPECT_TRUE(fsm.is_in_state<json_parser_fsm::context::end>());
}

TEST(Pushdown, PushObject)
{
    json_parser_fsm fsm;

    EXPECT_TRUE(fsm.is_in_state<json_parser_def>());
    EXPECT_TRUE(fsm.is_in_state<json_parser_fsm::context::start>());
    EXPECT_TRUE(done(fsm.process_event(events::start_object{})));

    EXPECT_TRUE(fsm.is_in_state<json_parser_fsm::context::object>());
    EXPECT_TRUE(fsm.is_in_state<json_parser_fsm::context::object::name>());
    EXPECT_TRUE(done(fsm.process_event(events::string_literal{})));
    EXPECT_TRUE(fsm.is_in_state<json_parser_fsm::context::object::colon>());
    EXPECT_TRUE(done(fsm.process_event(events::colon{})));

    EXPECT_TRUE(fsm.is_in_state<json_parser_fsm::context::start>());
    EXPECT_EQ(2, fsm.stack_size());

    EXPECT_FALSE(ok(fsm.process_event(events::end_object{})));
    EXPECT_TRUE(done(fsm.process_event(events::string_literal{})));
    EXPECT_EQ(1, fsm.stack_size());

    EXPECT_TRUE(done(fsm.process_event(events::comma{})));
    EXPECT_TRUE(done(fsm.process_event(events::string_literal{})));
    EXPECT_FALSE(ok(fsm.process_event(events::end_object{})));

    EXPECT_TRUE(done(fsm.process_event(events::colon{})));

    EXPECT_TRUE(fsm.is_in_state<json_parser_fsm::context::start>());
    EXPECT_EQ(2, fsm.stack_size());

    EXPECT_FALSE(ok(fsm.process_event(events::end_object{})));
    EXPECT_TRUE(done(fsm.process_event(events::string_literal{})));
    EXPECT_EQ(1, fsm.stack_size());

    EXPECT_TRUE(fsm.is_in_state< json_parser_fsm::context::object::value >());
    EXPECT_TRUE(done(fsm.process_event(events::end_object{})));
    EXPECT_TRUE(fsm.is_in_state< json_parser_fsm::context::end >());
}

TEST(Pushdown, EmptyObject)
{
    json_parser_fsm fsm;

    EXPECT_TRUE(fsm.is_in_state<json_parser_def>());
    EXPECT_TRUE(fsm.is_in_state<json_parser_fsm::context::start>());
    EXPECT_TRUE(done(fsm.process_event(events::start_object{})));

    EXPECT_TRUE(fsm.is_in_state<json_parser_fsm::context::object>());
    EXPECT_TRUE(fsm.is_in_state<json_parser_fsm::context::object::name>());

    EXPECT_TRUE(done(fsm.process_event(events::end_object{})));
    EXPECT_TRUE(fsm.is_in_state< json_parser_fsm::context::end >());
}

}  /* namespace test */
}  /* namespace afsm */

