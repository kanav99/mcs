#include <catch2/catch_test_macros.hpp>
#include <mcs.hpp>

TEST_CASE( "Commitment Scheme Works", "[mcs]" ) {
    hello_rust();
    mcs_gen_setup();
    mcs_load_setup();
    unsigned long long m[4];
    for (int i = 0; i < 4; i++) {
      m[i] = i;
    }
    mcs_gen_commit(2, 2, m);
    mcs_load_commit();

    unsigned long long x[2];
    x[0] = 1;
    x[1] = 2;
    unsigned long long y[2];
    y[0] = m[0] * x[0] + m[1] * x[1];
    y[1] = m[2] * x[0] + m[3] * x[1];
    REQUIRE( mcs_verify(2, 2, x, y) == 1 );

    y[1] = m[2] * x[0] + m[3] * x[1] + 5;
    REQUIRE( mcs_verify(2, 2, x, y) == 0 );
}
