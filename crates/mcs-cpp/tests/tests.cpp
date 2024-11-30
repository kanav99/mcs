#include <catch2/catch_test_macros.hpp>

#include <cstdint>

extern "C" {
  extern void hello_rust();
  extern void mcs_gen_setup();
  extern void mcs_load_setup();
  extern void mcs_gen_commit(uint d0, uint d1, unsigned long long *m);
  extern void mcs_load_commit();
  extern uint mcs_verify(uint d0, uint d1, unsigned long long *x, unsigned long long *y);
}

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
