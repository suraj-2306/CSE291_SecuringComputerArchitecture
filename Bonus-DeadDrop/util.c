#include "util.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

CYCLES measure_one_block_access_time(volatile ADDR_PTR addr)
{
    CYCLES cycles;
    asm volatile(
        "mov %1, %%r8\n\t"
        "lfence\n\t"
        "rdtsc\n\t"
        "mov %%eax, %%edi\n\t"
        "mov (%%r8), %%r8\n\t"
        "lfence\n\t"
        "rdtsc\n\t"
        "sub %%edi, %%eax\n\t"
        : "=a"(cycles)
        : "r"(addr)
        : "r8", "edi");
    return cycles;
}

void add_to_ll(volatile uint64_t addr, volatile struct Node **ll)
{
    if (*ll == NULL)
    {
        *ll = (struct Node *)malloc(sizeof(struct Node));
        assert(*ll != NULL);
        (*ll)->addr = addr;
        (*ll)->next = NULL;
        return;
    }
    volatile struct Node *nodeIter = *ll;
    while (nodeIter->next != NULL)
    {
        nodeIter = nodeIter->next;
    }
    struct Node *newNode = (struct Node *)malloc(sizeof(struct Node));
    assert(newNode != NULL);
    newNode->addr = addr;
    newNode->next = NULL;
    nodeIter->next = newNode;
}

void create_eviction_sets(volatile struct setup *setupStruct)
{
    // Initialize the eviction linked lists
    for (int i = 0; i < L2_SETS_COMM; i++)
    {
        setupStruct->eviction_ll[i] = NULL;
    }

    // Create eviction sets for all L2 cache sets
    int setidcount = 0;

    for (volatile int set = 0; set < L2_SETS; set++)
    {
        for (volatile int line = 0; line < L2_LINES_PER_SET; line++)
        {
            volatile uint64_t virt_addr =
                (volatile uint64_t)(setupStruct->eviction_buffer) +
                set * (L2_BYTES_PER_LINE * L2_LINES_PER_SET) +
                line * L2_BYTES_PER_LINE;

            volatile uint64_t set_index = get_set_index_from_virt_addr(virt_addr);

            if (set_index < 256)
            {
                if (set_index == 4)
                    setidcount++;
                add_to_ll(virt_addr, (volatile struct Node **)&(setupStruct->eviction_ll[set_index]));
            }
        }
    }
}

volatile uint64_t get_set_index_from_virt_addr(volatile uint64_t virt_addr)
{
    volatile uint64_t setNum =
        (virt_addr >> L2_BYTES_PER_LINE_LOG2) & (L2_SETS - 1);
    return setNum;
}
