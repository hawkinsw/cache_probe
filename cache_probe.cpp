#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <emmintrin.h>
#include <iostream>
#include <string.h>
#include <x86intrin.h>

using namespace std::chrono_literals;

const unsigned int ITERATIONS{100};
const unsigned MAX_CACHE_LINE_SIZE{256};

  alignas(64) volatile int results[MAX_CACHE_LINE_SIZE] = {0, };
  alignas(64) volatile int _[MAX_CACHE_LINE_SIZE] = {0, };
  alignas(64) volatile char data[MAX_CACHE_LINE_SIZE] = {0, };

void probe() {
  volatile register char read_base{0};
  volatile register char read_probe{0};
  unsigned int junk{0};
  int distance{0};
  volatile register int64_t before, after;
  unsigned int i{0};
  unsigned int mix{0};
  volatile char *addr;
  for (i = 0; i < ITERATIONS; i++) {
    for (distance = 0; distance < MAX_CACHE_LINE_SIZE; distance++) {
      mix = ((distance * 167) + 13) & 255;
      addr = &data[mix];
      _mm_clflush((void*)&data[0]);
      _mm_clflush((void*)&data[mix]);
      for (volatile int z = 0; z < 500; z++) {} /* Delay to let the cache fill. */
      read_base = data[0];
      before = __rdtscp(&junk);
      read_probe = *addr;
      after = __rdtscp(&junk);
      results[mix] += after - before;
    }
  }
  for (distance = 0; distance < MAX_CACHE_LINE_SIZE; distance++) {
    std::cout << distance << ", "
              << static_cast<double>(results[distance]) / ITERATIONS << "\n";
  }
}

int main() {
  probe();
  return 0;
}
