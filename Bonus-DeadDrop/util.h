
// You may only use fgets() to pull input from stdin
// You may use any print function to stdout to print
// out chat messages
#include <stdio.h>

// You may use memory allocators and helper functions
// (e.g., rand()).  You may not use system().
#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

#ifndef UTIL_H_
#define UTIL_H_

#define ADDR_PTR uint64_t
#define CYCLES uint32_t

#define L2_BYTES_PER_LINE 64
#define L2_BYTES_PER_LINE_LOG2 6
#define L2_LINES_PER_SET 4
#define L2_SETS 1024
#define L2_SETS_LOG2 10

#define BUFF_SIZE (1 << 21)

CYCLES measure_one_block_access_time(ADDR_PTR addr);

// You Should Not Use clflush in your final submission
// It is only used for debug
void clflush(ADDR_PTR addr);

char *string_to_binary(char *s);
char *binary_to_string(char *data);

int string_to_int(char *s);

struct Node
{
    uint64_t addr;     // address part
    struct Node *next; // Pointer to the next node
};

void add_to_ll(uint64_t addr, struct Node **ll);

uint64_t get_set_index_from_virt_addr(uint64_t virt_addr);

static inline uint64_t rdtscpp64()
{
    uint32_t low, high;
    asm volatile("rdtscpp" : "=a"(low), "=d"(high)::"ecx");
    return (((uint64_t)high) << 32) | low;
}

static inline uint64_t get_time()
{
    return rdtscpp64();
}

static inline uint64_t one_block_access(uint64_t addr)
{
    asm volatile("mov (%0), %%r8\n\t"
                 :           /*output*/
                 : "r"(addr) /*input*/
                 : "r8");    /*reserved register*/
}
struct setup
{
    uint8_t *eviction_buffer;

    struct Node *eviction_ll_0;
    struct Node *eviction_ll_1;

    int accessTimeWait;
    int primeTimeWait;
    int probeTimeWait;
};

struct Node *create_eviction_set(struct setup *setupStruct, int setNum);

#endif