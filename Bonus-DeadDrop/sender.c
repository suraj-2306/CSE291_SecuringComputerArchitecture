#include "util.h"
#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

void send_bit(volatile struct setup *setup, volatile uint8_t bit)
{
    volatile struct Node *currentNode;
    volatile int cacheLineAccess = 6;
    volatile int cacheLineAccessIter = 0;

    // Select the eviction set based on the bit value (set number)
    currentNode = setup->eviction_ll[bit];

    // Wait stage: Wait for the specified prime time before accessing cache lines
    volatile clock_t start_t = rdtsc();
    while ((rdtsc() - start_t) < setup->primeTimeWait)
    {
        asm volatile("" ::: "memory");
    }

    // Access stage: Access the cache lines in the selected eviction set
    start_t = rdtsc();
    while (currentNode != NULL &&
           (rdtsc() - start_t) < setup->accessTimeWait &&
           cacheLineAccessIter < cacheLineAccess)
    {
        // printf("iter %d\n", cacheLineAccessIter);
        cacheLineAccessIter++;
        volatile uint64_t *addr = (volatile uint64_t *)currentNode->addr;

        for (volatile int n = 0; n < 3; n++)
        {
            one_block_access((volatile uint64_t)addr);
        }

        currentNode = currentNode->next;
    }

    start_t = rdtsc();
    while ((rdtsc() - start_t) < setup->probeTimeWait)
    {
        asm volatile("" ::: "memory");
    }
}

void sender_config(volatile struct setup *setup)
{
    void *buf = mmap(NULL, BUFF_SIZE, PROT_READ | PROT_WRITE,
                     MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE | MAP_HUGETLB, -1, 0);
    if (buf == MAP_FAILED)
    {
        perror("mmap() error");
        exit(EXIT_FAILURE);
    }
    *((volatile char *)buf) = 1; // Trigger page allocation

    setup->eviction_buffer = buf;
    setup->primeTimeWait = PRIME_TIME;
    setup->probeTimeWait = PROBE_TIME;
    setup->accessTimeWait = ACCESS_TIME;

    // Create eviction sets for all 256 sets
    create_eviction_sets(setup);
}

int main()
{
    printf("Sender started.\n");

    volatile struct setup *setupStruct = (struct setup *)malloc(sizeof(struct setup));
    sender_config(setupStruct);

    uint8_t bit_to_send = 123; // Example bit (set number) to send

    while (1)
    {
        send_bit(setupStruct, bit_to_send);
    }

    printf("Sender finished.\n");
    return 0;
}
