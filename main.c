#include <stdio.h>
#include <pico/stdlib.h>

enum {
    FIBONACCI_INDEX = 66,
    UNROLL_FACTOR = 8
};

int main() {
    stdio_init_all();

    volatile const uint32_t chunk_count = (FIBONACCI_INDEX - 2) / 2 / UNROLL_FACTOR;

    while (true) {
        uint32_t chunks_remaining = chunk_count;

        uint64_t a = 1;
        uint64_t b = 1;

        const uint64_t start_us = time_us_64();

        do {
#pragma GCC unroll UNROLL_FACTOR
            for (uint32_t i = 0; i < UNROLL_FACTOR; ++i) {
                b += a;
                a += b;
            }
        } while (--chunks_remaining);

        const uint64_t end_us = time_us_64();
        const uint64_t duration_us = end_us - start_us;

        printf("Fibonaci #%d: %llu\n", FIBONACCI_INDEX, a);
        printf("Duration of %lu iterations: %llu microseconds\n", chunk_count, duration_us);

        sleep_ms(1000);
    }
}
