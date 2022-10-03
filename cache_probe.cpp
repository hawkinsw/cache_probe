#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>

const unsigned int ITERATIONS{500};

const unsigned MAX_CACHE_LINE_SIZE{2048};

inline void flush(volatile char *ptr) {
  asm volatile("clflush %0\n" ::"m"(*ptr));
}

void probe() {
  volatile char read_base{0};
  volatile char read_probe{0};

  std::chrono::nanoseconds duration{0};
  alignas(uint64_t) volatile char data[MAX_CACHE_LINE_SIZE * sizeof(char)];

  for (unsigned int distance{0}; distance < MAX_CACHE_LINE_SIZE; distance++) {
    auto before = std::chrono::high_resolution_clock::now();
    for (unsigned int i = 0; i < ITERATIONS; i++) {
      flush(&data[0]);
      read_base = data[0];
      read_probe = data[distance];
    }
    auto after = std::chrono::high_resolution_clock::now();
    std::chrono::nanoseconds difference = after - before;
    std::cout << distance << ", "
              << static_cast<double>(difference.count()) / ITERATIONS << "\n";
  }
}

int main() {
  probe();
  return 0;
}
