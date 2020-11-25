/**
 * Copyright 2019 Sergei A. Fedorov
 * dot_diagram.cpp
 *
 *  Created on: Mar 16, 2019
 *      Author: ser-fedorov
 */

#include <afsm/dot.hpp>

#include <gtest/gtest.h>

#include <sstream>

namespace afsm::test {
using dot::operator""_string;

constexpr auto foobar = "foobar"_string;
constexpr auto foo    = "foo"_string;
constexpr auto bar    = "bar"_string;

// static_assert(begins_with(foobar, foo), "");

TEST(CT_STRING, Output)
{
    auto               cs = "foobar"_string;
    std::ostringstream os;
    os << cs;
    EXPECT_EQ("foobar", os.str());
}
}    // namespace afsm::test
