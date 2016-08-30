/*
 * static_tests.cpp
 *
 *  Created on: May 31, 2016
 *      Author: zmij
 */

#include <gtest/gtest.h>
#include <pushkin/meta.hpp>

namespace psst {
namespace meta {
namespace test {

static_assert(!contains<int, void>::value, "");

static_assert( ::std::is_same<
        combine< int, type_tuple< long, double > >::type,
        type_tuple< int, long, double >
    >::value, "");

static_assert( ::std::is_same<
        combine< int, type_tuple< long, double >, float >::type,
        type_tuple< int, long, double, float >
    >::value, "");

static_assert( ::std::is_same<
        combine< int, type_tuple< long, double >, float, type_tuple< bool > >::type,
        type_tuple< int, long, double, float, bool >
    >::value, "");

static_assert( all_match< ::std::is_fundamental, int, float, double, long >::value, "" );
static_assert( !all_match< ::std::is_integral, int, float, double, long >::value, "" );

static_assert( find_if< ::std::is_integral, int, float, double, long >::type::size == 2, "");
static_assert( find_if< ::std::is_integral, int, float, double, long, long double >::type::size == 2, "");
static_assert(
    ::std::is_same<
        find_if< ::std::is_integral, int, float, double, long >::type,
        type_tuple<int, long>
     >::value, "");

static_assert( ::std::is_same<insert_type<int, float, bool>::type,
        type_tuple<int, float, bool>>::value, "" );
static_assert( ::std::is_same<insert_type<int, int, float, bool>::type,
        type_tuple<int, float, bool>>::value, "" );

static_assert( ::std::is_same<insert_type<type_tuple<float, bool>, int>::type,
        type_tuple<int, float, bool>>::value, "" );
static_assert( ::std::is_same<insert_type<type_tuple<int, float, bool>, int>::type,
        type_tuple<int, float, bool>>::value, "" );
using type_set_1 = unique< int, float, int, double, long, float, double, bool >::type;
static_assert( contains< int, type_set_1 >::value, "" );
static_assert( contains< float, type_set_1 >::value, "" );
static_assert( contains< double, type_set_1 >::value, "" );
static_assert( contains< long, type_set_1 >::value, "" );
static_assert( contains< bool, type_set_1 >::value, "" );


struct test_callable {
    template < typename A, typename B >
    void
    operator() (A&, B&&);

    void
    operator() (int, int) const;
};

static_assert(is_callable<test_callable, int, double>::value, "");
static_assert(is_callable<test_callable, int, int>::value, "");
static_assert(!is_callable<test_callable, int>::value, "");


TEST(Dummy, AllIsDoneStatically)
{
}

}  /* namespace test */
}  /* namespace meta */
}  /* namespace pus */
