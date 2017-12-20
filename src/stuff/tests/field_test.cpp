#define BOOST_TEST_MODULE Field

#include <boost/test/unit_test.hpp>

#include "../field.h"

BOOST_AUTO_TEST_CASE(Dimensions)
{
    Field<int> f;
    BOOST_REQUIRE_EQUAL(f.width(), 0);
    BOOST_REQUIRE_EQUAL(f.height(), 0);

    f.reset(1, 0);
    BOOST_REQUIRE_EQUAL(f.width(), 1);
    BOOST_REQUIRE_EQUAL(f.height(), 0);

    f.reset(0, 1);
    BOOST_REQUIRE_EQUAL(f.width(), 0);
    BOOST_REQUIRE_EQUAL(f.height(), 1);

    f.reset(2, 3);
    BOOST_REQUIRE_EQUAL(f.width(), 2);
    BOOST_REQUIRE_EQUAL(f.height(), 3);

    f.reset(3, 2);
    BOOST_REQUIRE_EQUAL(f.width(), 3);
    BOOST_REQUIRE_EQUAL(f.height(), 2);
}

BOOST_AUTO_TEST_CASE(AtRangeException)
{
    Field<int> f(3, 5);
    BOOST_REQUIRE_THROW(f.at(0, 9) = 0, std::out_of_range);
    BOOST_REQUIRE_THROW(f.at(9, 0) = 0, std::out_of_range);
    BOOST_REQUIRE_THROW(f.at(9, 9) = 0, std::out_of_range);
}

BOOST_AUTO_TEST_CASE(ValuesAtAccess)
{
    Field<int> f(3, 5);
    for( size_t i = 0; i < f.width() * f.height(); i++ )
    {
        BOOST_REQUIRE_NO_THROW(f.at(i % f.width(), i / f.width()) = i);
    }
    for( size_t i = 0; i < f.width() * f.height(); i++ )
    {
        BOOST_REQUIRE_EQUAL(f.at(i % f.width(), i / f.width()), i);
    }
}

BOOST_AUTO_TEST_CASE(ArrayRangeException)
{
    Field<int> f(3, 5);
    BOOST_REQUIRE_THROW(f[9], std::out_of_range);
}

BOOST_AUTO_TEST_CASE(ValuesArrayAccess)
{
    Field<int> f(3, 5);
    for( size_t i = 0; i < f.width() * f.height(); i++ )
    {
        BOOST_REQUIRE_NO_THROW(f[i / f.width()][i % f.width()] = i);
    }
    for( size_t i = 0; i < f.width() * f.height(); i++ )
    {
        BOOST_REQUIRE_EQUAL(f[i / f.width()][i % f.width()], i);
    }
}