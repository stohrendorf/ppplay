#define BOOST_TEST_MODULE PPG
#include <boost/test/unit_test.hpp>

#include "../point.h"
#include "../rect.h"

BOOST_AUTO_TEST_SUITE( Point )
BOOST_AUTO_TEST_CASE( PointArithmetic )
{
    ppg::Point p;
    BOOST_CHECK_EQUAL( p.x(), 0 );
    BOOST_CHECK_EQUAL( p.y(), 0 );

    p += ppg::Point( 2, 3 );
    BOOST_CHECK_EQUAL( p.x(), 2 );
    BOOST_CHECK_EQUAL( p.y(), 3 );

    p.setX( 9 );
    BOOST_CHECK_EQUAL( p.x(), 9 );

    p.setY( -10 );
    BOOST_CHECK_EQUAL( p.y(), -10 );

    p -= ppg::Point( 10, -10 );
    BOOST_CHECK_EQUAL( p.x(), -1 );
    BOOST_CHECK_EQUAL( p.y(), 0 );
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( Rect )
BOOST_AUTO_TEST_CASE( RectDims )
{
    ppg::Rect r( 1, 1, 1, 1 );
    BOOST_CHECK_EQUAL( r.top(), 1 );
    BOOST_CHECK_EQUAL( r.left(), 1 );
    BOOST_CHECK_EQUAL( r.bottom(), 1 );
    BOOST_CHECK_EQUAL( r.right(), 1 );
    BOOST_CHECK_EQUAL( r.width(), 1 );
    BOOST_CHECK_EQUAL( r.height(), 1 );

    BOOST_CHECK( r.contains( 1, 1 ) ); // inner
    BOOST_CHECK( !r.contains( 1, 0 ) ); // top
    BOOST_CHECK( !r.contains( 0, 1 ) ); // left
    BOOST_CHECK( !r.contains( 1, 2 ) ); // bottom
    BOOST_CHECK( !r.contains( 2, 1 ) ); // right
    BOOST_CHECK( !r.contains( 0, 0 ) ); // top left
    BOOST_CHECK( !r.contains( 0, 2 ) ); // bottom left
    BOOST_CHECK( !r.contains( 2, 0 ) ); // top right
    BOOST_CHECK( !r.contains( 2, 2 ) ); // bottom right

    r = ppg::Rect( 0, 0, 1, 1 );
    BOOST_CHECK_EQUAL( r.top(), 0 );
    BOOST_CHECK_EQUAL( r.left(), 0 );
    BOOST_CHECK_EQUAL( r.bottom(), 0 );
    BOOST_CHECK_EQUAL( r.right(), 0 );
    BOOST_CHECK_EQUAL( r.width(), 1 );
    BOOST_CHECK_EQUAL( r.height(), 1 );

    r = ppg::Rect( 0, 0, 3, 4 );
    BOOST_CHECK_EQUAL( r.top(), 0 );
    BOOST_CHECK_EQUAL( r.left(), 0 );
    BOOST_CHECK_EQUAL( r.bottom(), 3 );
    BOOST_CHECK_EQUAL( r.right(), 2 );
    BOOST_CHECK_EQUAL( r.width(), 3 );
    BOOST_CHECK_EQUAL( r.height(), 4 );
}
BOOST_AUTO_TEST_SUITE_END()
