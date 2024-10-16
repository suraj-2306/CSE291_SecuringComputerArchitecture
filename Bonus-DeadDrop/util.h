#ifndef UTIL_H_
#define UTIL_H_

#include <stdint.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>

#define ADDR_PTR uint64_t
#define CYCLES uint32_t

#define L2_BYTES_PER_LINE 64
#define L2_BYTES_PER_LINE_LOG2 6
#define L2_LINES_PER_SET 4 // Assuming 8-way associativity
#define L2_SETS 1024
#define L2_SETS_COMM 256
#define BUFF_SIZE (1 << 22) // Adjusted buffer size for more sets
#define PRIME_TIME 0x000800000
#define PROBE_TIME 0x000800000
#define ACCESS_TIME 0x0f00000

struct Node
{
    volatile uint64_t addr;
    volatile struct Node *next;
};

struct setup
{
    volatile uint64_t *eviction_buffer;
    volatile struct Node *eviction_ll[L2_SETS_COMM]; // Array of eviction sets
    volatile int accessTimeWait;
    volatile int primeTimeWait;
    volatile int probeTimeWait;
};

static inline uint64_t rdtsc()
{
    uint64_t a, d;
    asm volatile("lfence");
    asm volatile("rdtsc" : "=a"(a), "=d"(d));
    asm volatile("lfence");
    return (d << 32) | a;
}
static inline void one_block_access(volatile uint64_t addr)
{
    asm volatile("mov (%0), %%r8\n\t"
                 :
                 : "r"(addr)
                 : "r8", "memory");
}

// Function declarations
static inline void one_block_access(volatile uint64_t addr);

CYCLES measure_one_block_access_time(volatile ADDR_PTR addr);
void add_to_ll(volatile uint64_t addr, volatile struct Node **ll);
void create_eviction_sets(volatile struct setup *setupStruct);
volatile uint64_t get_set_index_from_virt_addr(volatile uint64_t virt_addr);

#endif // UTIL_H_
