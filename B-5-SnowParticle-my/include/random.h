#pragma once

/*
  No pseudo-random generator function is perfect, but some are useful.
  DO NOT use these code for cryptographic purposes.
  Seed is modified to be fast and easy to feed.
*/

#include <time.h>
#include <stdlib.h>
#include <stdint.h>

#define __UINT64_MAX 0xFFFFFFFFFFFFFFFFULL
#define __UINT64_INVALID __UINT64_MAX

#if defined(ARCH_X64)
static inline uint64_t __seed()
{
    uint32_t lo, hi;
    __asm__ __volatile__("rdtsc" : "=a" (lo), "=d" (hi));
    uint64_t t = lo;
    return (t << 32) | (t & 0xAAAA5555); /* A(1010)5(0101) won't INVALID(1111_1111) again */
}
#elif defined(ARCH_AARCH64)
static inline uint64_t __seed()
{
    uint64_t t;
    __asm__ __volatile__("mrs %0, CNTVCT_EL0" : "=r"(t));
    return (t << 32) | (t & 0xAAAA5555);
}
#elif defined(MYS_FAKE_RANDOM)
static inline uint64_t __seed()
{
    return (uint64_t)1;
}
#else
static inline uint64_t __seed()
{
    uint64_t t = (uint64_t)time(NULL);
    return (t << 32) | (t & 0xAAAA5555);
}
#endif


/* legacy random */
static uint64_t __legacy_x = __UINT64_INVALID;
static inline void __legacy_init(uint64_t seed)
{
    __legacy_x = seed;
    srand((unsigned int)__legacy_x);
}
static inline uint64_t __legacy_rand()
{
    if (__legacy_x == __UINT64_INVALID)
        __legacy_init(__seed());
    double perc = (double)rand() / (double)RAND_MAX;
    return perc * (double)__UINT64_MAX;
}


/* https://prng.di.unimi.it
 * https://prng.di.unimi.it/splitmix64.c
 * This is a fixed-increment version of Java 8's SplittableRandom generator
 * See http://dx.doi.org/10.1145/2714064.2660195 and 
 * http://docs.oracle.com/javase/8/docs/api/java/util/SplittableRandom.html
 * It is a very fast generator passing BigCrush, and it can be useful if
 * for some reason you absolutely want 64 bits of state.
 */
static uint64_t __splitmix64_x = 0; /* The state can be seeded with any value. */
static inline void __splitmix_init(uint64_t seed)
{
    __splitmix64_x = seed;
}
static inline uint64_t __splitmix_rand()
{
    if (__splitmix64_x == __UINT64_INVALID)
        __splitmix_init(__seed());
    uint64_t z = (__splitmix64_x += 0x9e3779b97f4a7c15);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
    z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
    return z ^ (z >> 31);
}


/* https://prng.di.unimi.it
 * https://prng.di.unimi.it/xoroshiro128starstar.c
 * This is xoroshiro128** 1.0, one of our all-purpose, rock-solid,
 * small-state generators. It is extremely (sub-ns) fast and it passes all
 * tests we are aware of, but its state space is large enough only for
 * mild parallelism.
 * For generating just floating-point numbers, xoroshiro128+ is even
 * faster (but it has a very mild bias, see notes in the comments).
 * The state must be seeded so that it is not everywhere zero. If you have
 * a 64-bit seed, we suggest to seed a splitmix64 generator and use its
 * output to fill __xoroshiro128_x.
 */
static uint64_t __xoroshiro128_x[2] = {__UINT64_INVALID, __UINT64_INVALID};
static inline void __xoroshiro128ss_init(uint64_t seed)
{
    __xoroshiro128_x[0] = seed;
    __xoroshiro128_x[1] = __UINT64_MAX - seed;
}
static inline uint64_t __rotl(const uint64_t x, int k)
{
    return (x << k) | (x >> (64 - k));
}
static inline uint64_t __xoroshiro128ss_rand()
{
    if (__xoroshiro128_x[0] == __UINT64_INVALID)
        __xoroshiro128ss_init(__seed()); /* Suggest to use __splitmix_rand() */
    const uint64_t s0 = __xoroshiro128_x[0];
    uint64_t s1 = __xoroshiro128_x[1];
    const uint64_t result = __rotl(s0 * 5, 7) * 9;
    s1 ^= s0;
    __xoroshiro128_x[0] = __rotl(s0, 24) ^ s1 ^ (s1 << 16);
    __xoroshiro128_x[1] = __rotl(s1, 37);
    return result;
}


static inline const char *randname()
{
#if defined(MYS_LEGACY_RANDOM)
    return "legacy random generator";
#elif defined(MYS_SPLITMIX_RANDOM)
    return "splitmix random generator";
#else
    return "xoroshiro128ss random generator";
#endif
}

/**
 * @brief Generate random uint64_t value
 * 
 * @param minimum minimum value (inclusive)
 * @param maximum minimum value (inclusive)
 * @return uint64_t random value between [minimum, maximum]
 */
static inline uint64_t randu64(uint64_t minimum, uint64_t maximum)
{
#if defined(MYS_LEGACY_RANDOM)
    uint64_t rand = __legacy_rand();
#elif defined(MYS_SPLITMIX_RANDOM)
    uint64_t rand = __splitmix_rand();
#else
    uint64_t rand = __xoroshiro128ss_rand();
#endif
    if (maximum <= minimum)
        return minimum;
    if (minimum == 0 && maximum == __UINT64_MAX)
        return rand;
    double range = maximum - minimum + 1;
    double perc = (double)rand / (double)__UINT64_MAX;
    return perc * range;
}

static inline int64_t randi64(int64_t minimum, int64_t maximum)
{
    uint64_t _min = (uint64_t)minimum;
    uint64_t _max = (uint64_t)maximum;
    uint64_t result = randu64(_min, _max);
    return *(int64_t *)((void *)&result);
}

static inline double randf64(double minimum, double maximum)
{
    double range = maximum - minimum;
    double div = (double)__UINT64_MAX / range;
    return minimum + (randu64(0, __UINT64_MAX) / div);
}

static inline uint32_t randu32(uint32_t minimum, uint32_t maximum)
{
    return (uint32_t)randu64(minimum, maximum);
}

static inline int32_t randi32(int32_t minimum, int32_t maximum)
{
    return (int32_t)randi64(minimum, maximum);
}

static inline float randf32(float minimum, float maximum)
{
    return (float)randf64(minimum, maximum);
}

#undef __UINT64_INVALID
#undef __UINT64_MAX

/* Tester:
- mpicc -O3 -g -lm -I../include xoshiro256.c && ./a.out 1000000000 > xoshiro256.log

- Intel Xeon Gold 6132 (icc 19.0.5.281)
    - xoroshiro128ss: 1000000000 double in 4.33 sec (0.54 ns/Byte 1.85 GB/s)
    - splitmix:       1000000000 double in 5.78 sec (0.72 ns/Byte 1.38 GB/s)
    - legacy:         1000000000 double in 15.24 sec (1.91 ns/Byte 0.52 GB/s)
- Huawei Kunpeng920 (gcc 9.3.1):
    - xoroshiro128ss: 1000000000 double in 7.42 sec (0.93 ns/Byte 1.08 GB/s)
    - splitmix:       1000000000 double in 7.42 sec (0.93 ns/Byte 1.08 GB/s)
    - legacy:         1000000000 double in 24.04 sec (3.01 ns/Byte 0.33 GB/s)
- Apple M1 (Apple clang version 14.0.0):
    - xoroshiro128ss: 1000000000 double in 6.11 sec (0.76 ns/Byte 1.31 GB/s)
    - splitmix:       1000000000 double in 3.11 sec (0.39 ns/Byte 2.57 GB/s)
    - legacy:         1000000000 double in 8.74 sec (1.09 ns/Byte 0.92 GB/s)

// #define MYS_SPLITMIX_RANDOM
// #define MYS_LEGACY_RANDOM
#include <mys.h>

#define minimum 0
#define maximum 100
#define ssize ((maximum)-(minimum)+1)

int main(int argc , char **argv) {
    uint64_t n = 1000000000;
    if (argc > 1) {
        n = atoll(argv[1]);
    }
    uint64_t box[ssize] = {0};
    printf("%s\n", randname());
    double *buffer = (double *)malloc(n * sizeof(double));
    memset(buffer, 0, n * sizeof(double));
    double t1 = hrtime();
    for (uint64_t i = 0; i < n; i++) {
        buffer[i] = randf64(minimum, maximum);
    }
    double t2 = hrtime();
    double tdiff = t2 - t1;
    double ns = (tdiff * 1e9)/(double)(n * sizeof(double));
    double throughput = 1 / ns;
    printf("%lld double in %.2f sec (%.2f ns/Byte %.2f GB/s)\n", n, tdiff, ns, throughput);

    for (uint64_t i = 0; i < n; i++) {
        box[(int)buffer[i] - minimum] += 1;
    }

    for (int i = 0; i < ssize; i++) {
        printf("[%03d,%03d) %.3f %d\n", i-minimum, i+1-minimum, ((double)box[i]/n) * 100, box[i]);
    }

    free(buffer);
}


*/
