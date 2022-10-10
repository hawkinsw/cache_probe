#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string.h>
#ifndef ARM
#include <x86intrin.h>
#endif

const unsigned int ITERATIONS{100};
/*
 * Can only go up to 256 because of the *funky*
 * way that we are striding through these values
 * so that the CPU cannot predict where we are
 * going.
 */
const unsigned MAX_CACHE_LINE_SIZE{256};
const unsigned MAX_LL_CACHE_SIZE{9 * 1024 * 1024};

alignas(64) volatile int results[MAX_CACHE_LINE_SIZE] = {0, };
/*
 * This will pad between the two arrays so that a write to
 * results does not stomp on a write to data. We will need
 * to make sure that they do not alias the same cache line.
 * Left as an "exercise for the reader."
 */
alignas(64) volatile int _[MAX_CACHE_LINE_SIZE] = {0, };
alignas(64) volatile char data[MAX_CACHE_LINE_SIZE] = {0, };
alignas(64) volatile int __[MAX_CACHE_LINE_SIZE] = {0, };
/* 
 * We are going to use this to forcefully clean the entire cache.
 */
alignas(64) volatile char reset[MAX_LL_CACHE_SIZE] = {0, };

void probe() {
  // Mark these as volatile so accesses are not optimized by compiler.
  register char read_base{0};
  register char read_probe{0};
  register int64_t before, after;
  int distance{0};
  unsigned int junk{0};
  unsigned int i{0};
  unsigned int mix{0};
  volatile char *addr;

#ifdef ARM
  // If we are on ARM, we want to enable the 0th performance
  // counter register to count the L1D_CACHE_REFILL_RD (0x42) event.
  asm volatile ("msr pmevtyper0_el0, %0": : "r"(0x66):);
#endif

  for (i = 0; i < ITERATIONS; i++) {
    for (distance = 0; distance < MAX_CACHE_LINE_SIZE; distance++) {
      mix = ((distance * 167) + 13) & 255;
      // Precalculate the address from which to read so that what we measure
      // is pure access.
      addr = &data[mix];
      // Flush the caches every. single. time. Won't get fooled again.
#ifndef ARM
      _mm_clflush((void*)&data[0]);
      _mm_clflush((void*)&data[mix]);
      for (volatile int z = 0; z < 500; z++) {} //Delay to let the flush happen
#else
      memset((void*)reset, 0, MAX_LL_CACHE_SIZE);
#endif



      read_base = data[0];
#ifdef ARM
      asm volatile ("dmb ishld \n mrs %0, pmevcntr0_el0": "=r" (before):);
#else
      // Use the p variant of the rdtsc instruction for ordering purposes.
      before = __rdtscp(&junk);
#endif

      read_probe = *addr;

#ifdef ARM
      asm volatile ("\n mrs %0, pmevcntr0_el0": "=r" (after):);
#else
      after = __rdtscp(&junk);
#endif
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
