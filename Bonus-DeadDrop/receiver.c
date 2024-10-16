#include "util.h"
#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int receive_bit(volatile struct setup *setup)
{

    // Prime stage:
    int cacheLineAccess = 9;
    volatile uint64_t start_t = rdtsc();
    int set = 0;
    while ((rdtsc() - start_t) < setup->primeTimeWait && set < L2_SETS_COMM)
    // for (int set = 0; set < L2_SETS_COMM; set++)
    {
        volatile struct Node *currentNode = setup->eviction_ll[set];
        int cacheLineAccessIter = 0;

        while (currentNode != NULL && cacheLineAccessIter < cacheLineAccess)
        {
            volatile uint64_t addr = currentNode->addr;
            for (volatile int n = 0; n < 3; n++)
            {
                one_block_access((volatile uint64_t)addr);
            }
            // printf("set no %d, cachelineIter %d\n", set, cacheLineAccessIter);
            currentNode = currentNode->next;
            cacheLineAccessIter++;
        }
        set++;
    }

    // Wait stage: Align with sender's access stage
    start_t = rdtsc();
    while ((rdtsc() - start_t) < setup->accessTimeWait)
    {
        asm volatile("" ::: "memory");
    }

    // Probe stage: Measure access times for all sets
    int detected_set = -1;
    cacheLineAccess = 4;
    CYCLES accessTimes[L2_SETS_COMM][cacheLineAccess];
    start_t = rdtsc();
    set = 0;
    while ((rdtsc() - start_t) < setup->probeTimeWait && set < L2_SETS_COMM)
    {
        volatile struct Node *currentNode = setup->eviction_ll[set];
        int cacheLineAccessIter = 0;

        while (currentNode != NULL && cacheLineAccessIter < cacheLineAccess)
        {
            volatile uint64_t addr = currentNode->addr;
            CYCLES time = measure_one_block_access_time(addr);
            accessTimes[set][cacheLineAccessIter] = time;
            // printf("set no %d, cachelineIter %d\n", set, cacheLineAccessIter);
            currentNode = currentNode->next;
            cacheLineAccessIter++;
        }
        // printf("set %d", set);
        set++;
    }

    // Analyze access times to find the set with 4 misses
    for (int set = 0; set < L2_SETS_COMM; set++)
    {
        int misses = 0;
        for (int i = 0; i < cacheLineAccess; i++)
        {
            if (accessTimes[set][i] > 48) // Threshold for miss
            {
                misses++;
            }
        }
        if (misses == cacheLineAccess)
        {
            // Found the set that was accessed
            detected_set = set;
            // printf("%d %d %d %d\n", accessTimes[set][0], accessTimes[set][1], accessTimes[set][2], accessTimes[set][3]);
            // break;
        }
    }

    return detected_set;
}

void receiver_config(volatile struct setup *setup)
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

    // Create eviction sets for all 256 sets
    create_eviction_sets(setup);
    setup->primeTimeWait = PRIME_TIME;
    setup->probeTimeWait = PROBE_TIME;
    setup->accessTimeWait = ACCESS_TIME;
}

int main()
{
    printf("Receiver now listening.\n");

    volatile struct setup *setupStruct = (struct setup *)malloc(sizeof(struct setup));
    receiver_config(setupStruct);

    while (1)
    {
        int detected_set = receive_bit(setupStruct);
        if (detected_set != -1)
            printf("Detected set accessed: %d\n", detected_set);
    }

    printf("Receiver finished.\n");
    return 0;
}
