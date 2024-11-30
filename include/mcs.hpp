#pragma once

#include <cstdint>
#include <iostream>

extern "C" {
  extern void hello_rust();
  extern void mcs_gen_setup();
  extern void mcs_load_setup();
  extern void mcs_gen_commit(uint d0, uint d1, unsigned long long *m);
  extern void mcs_load_commit();
  extern uint mcs_verify(uint d0, uint d1, unsigned long long *x, unsigned long long *y);
}
