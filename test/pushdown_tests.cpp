/*
 * pushdown_tests.cpp
 *
 *  Created on: Nov 29, 2016
 *      Author: zmij
 */

#include <gtest/gtest.h>

#include <afsm/fsm.hpp>

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

struct json_parser_def : def::state_machine< json_parser_def > {
    struct start : state<start> {};

    struct array : state_machine< array > {
        struct value : push< value, json_parser_def > {};

        using initial_state = value;

        using transitions = transition_table<
            tr< value,  events::comma,          value >
        >;
    };

    struct object : state_machine< object > {
        struct name : state<name> {};
        struct colon : state< colon > {};
        struct value : push< value, json_parser_def > {};

        using initial_state = name;

        using transitions = transition_table<
            tr< name,   events::string_literal, colon   >,
            tr< colon,  events::colon,          value   >,
            tr< value,  events::comma,          name    >
        >;
    };

    struct end : pop< end, json_parser_def > {};

    using initial_state = start;
    using transitions = transition_table<
        tr< start,  events::bool_literal,       end     >,
        tr< start,  events::integer_literal,    end     >,
        tr< start,  events::float_literal,      end     >,
        tr< start,  events::string_literal,     end     >,
        tr< start,  events::null_literal,       end     >,

        tr< start,  events::start_array,        array   >,
        tr< array,  events::end_array,          end     >,

        tr< start,  events::start_object,       object  >,
        tr< object, events::end_object,         end     >
    >;
};

using json_parser_fsm = state_machine<json_parser_def>;

static_assert(def::traits::is_state<json_parser_def::array::value>::value, "");
static_assert(def::traits::is_pushdown<json_parser_def::array::value>::value, "");
static_assert(def::traits::is_pushdown<json_parser_def::object::value>::value, "");
static_assert(def::traits::is_popup<json_parser_def::end>::value, "");

static_assert(def::traits::pushes<json_parser_def::array::value, json_parser_def>::value, "");
static_assert(def::traits::pushes<json_parser_def::object::value, json_parser_def>::value, "");

static_assert(def::state_pushes<json_parser_def::array::value, json_parser_def>::value, "");
static_assert(def::state_pushes<json_parser_def::object::value, json_parser_def>::value, "");

static_assert(def::traits::pops<json_parser_def::end, json_parser_def>::value, "");
static_assert(def::state_pops<json_parser_def::end, json_parser_def>::value, "");

static_assert(def::contains_pushdowns<json_parser_def::array>::value, "");
static_assert(def::contains_pushdowns<json_parser_def::object>::value, "");
static_assert(!def::contains_popups<json_parser_def::array>::value, "");
static_assert(!def::contains_popups<json_parser_def::object>::value, "");

static_assert(def::contains_pushdowns<json_parser_def>::value, "");
static_assert(def::contains_popups<json_parser_def>::value, "");

static_assert(def::is_pushed<json_parser_def>::value, "");
static_assert(!def::is_pushed<json_parser_def::array>::value, "");
static_assert(!def::is_pushed<json_parser_def::object>::value, "");

static_assert(def::is_popped<json_parser_def>::value, "");
static_assert(!def::is_popped<json_parser_def::array>::value, "");
static_assert(!def::is_popped<json_parser_def::object>::value, "");

static_assert(def::has_pushdown_stack<json_parser_def>::value, "");
static_assert(!def::has_pushdown_stack<json_parser_def::array>::value, "");
static_assert(!def::has_pushdown_stack<json_parser_def::object>::value, "");

static_assert(::std::is_same<
        def::state_path<json_parser_def, json_parser_def>::type,
        ::psst::meta::type_tuple<json_parser_def>
    >::value, "");
static_assert(::std::is_same<
        def::state_path<json_parser_def::start, json_parser_def::start>::type,
        ::psst::meta::type_tuple<json_parser_def::start>
    >::value, "");
static_assert(::std::is_same<
        def::state_path<json_parser_def, json_parser_def::start>::type,
        ::psst::meta::type_tuple<json_parser_def, json_parser_def::start>
    >::value, "");

static_assert(::std::is_same<
        def::state_path<json_parser_def, json_parser_def::object::name>::type,
        ::psst::meta::type_tuple<
             json_parser_def,
             json_parser_def::object,
             json_parser_def::object::name
        >
    >::value, "");

static_assert(::std::is_same<
        def::state_path<json_parser_fsm, json_parser_def::object::name>::type,
        ::psst::meta::type_tuple<
             json_parser_def,
             json_parser_def::object,
             json_parser_def::object::name
        >
    >::value, "");

TEST(Pushdown, OneDown)
{
    json_parser_fsm fsm;

    EXPECT_TRUE(fsm.is_in_state<json_parser_def>());
    EXPECT_TRUE(fsm.is_in_state<json_parser_fsm::start>());
    //EXPECT_TRUE(done(fsm.process_event(events::bool_literal{})));
}

}  /* namespace test */
}  /* namespace afsm */

