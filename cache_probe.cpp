#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <x86intrin.h>

const unsigned int ITERATIONS{100};
/*
 * Can only go up to 256 because of the *funky*
 * way that we are striding through these values
 * so that the CPU cannot predict where we are
 * going.
 */
const unsigned MAX_CACHE_LINE_SIZE{256};

alignas(64) volatile int results[MAX_CACHE_LINE_SIZE] = {0, };
/*
 * This will pad between the two arrays so that a write to
 * results does not stomp on a write to data. We will need
 * to make sure that they do not alias the same cache line.
 * Left as an "exercise for the reader."
 */
alignas(64) volatile int _[MAX_CACHE_LINE_SIZE] = {0, };
alignas(64) volatile char data[MAX_CACHE_LINE_SIZE] = {0, };

void probe() {
  // Mark these as volatile so accesses are not optimized by compiler.
  volatile register char read_base{0};
  volatile register char read_probe{0};
  volatile register int64_t before, after;
  int distance{0};
  unsigned int junk{0};
  unsigned int i{0};
  unsigned int mix{0};
  volatile char *addr;
  for (i = 0; i < ITERATIONS; i++) {
    for (distance = 0; distance < MAX_CACHE_LINE_SIZE; distance++) {
      mix = ((distance * 167) + 13) & 255;
      // Precalculate the address from which to read so that what we measure
      // is pure access.
      addr = &data[mix];
      // Flush the caches every. single. time. Won't get fooled again.
      _mm_clflush((void*)&data[0]);
      _mm_clflush((void*)&data[mix]);
      for (volatile int z = 0; z < 500; z++) {} //Delay to let the flush happen
      read_base = data[0];
			// Use the p variant of the rdtsc instruction for ordering purposes.
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
