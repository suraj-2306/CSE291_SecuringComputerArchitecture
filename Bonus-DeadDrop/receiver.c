#include "util.h"
#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

// Function to receive a bit by performing cache operations and detecting misses
// It takes the setup structure and an array of misses per set as inputs
// It performs three stages: Prime, Wait, and Probe, and returns the detected set
uint64_t receive_bit(volatile struct setup *setup, int missesArray[])
{
    int cacheLineAccess = 5; // Number of cache lines to access per set

    // Prime stage: Access cache lines in each set to fill the cache
    int set = 0;
    while (set < L2_SETS_COMM)
    {
        volatile struct Node *currentNode = setup->eviction_ll[set]; // Start from the first node in the eviction set
        int cacheLineAccessIter = 0;

        // Access cache lines in the eviction list of the current set
        while (currentNode != NULL && cacheLineAccessIter < cacheLineAccess)
        {
            volatile uint64_t addr = currentNode->addr; // Get the address of the current cache line
            one_block_access((volatile uint64_t)addr);  // Access the cache line to prime the set
            currentNode = currentNode->next;            // Move to the next cache line in the eviction list
            cacheLineAccessIter++;
        }
        set++;
    }

    // Wait stage: Wait for the sender's access stage to complete by aligning the receiver's timing
    volatile uint64_t start_t = rdtsc();
    while ((rdtsc() - start_t) < setup->accessTimeWait)
    {
        asm volatile("" ::: "memory"); // Ensure no compiler optimizations disrupt the timing wait
    }

    // Probe stage: Measure access times for each set to detect cache misses
    int detected_set = 0;
    cacheLineAccess = 4;                               // Adjust the number of cache lines to access during probing
    CYCLES accessTimes[L2_SETS_COMM][cacheLineAccess]; // Array to store access times
    int returnSet = -1;                                // Default value when no set is detected
    int bitFlag = 0;                                   // Flag to indicate if a set is detected

    // Iterate through all sets and measure access times
    for (int set = 0; set < L2_SETS_COMM; set++)
    {
        volatile struct Node *currentNode = setup->eviction_ll[set];
        int cacheLineAccessIter = 0;

        // Measure access times for cache lines in the current eviction set
        while (currentNode != NULL && cacheLineAccessIter < cacheLineAccess)
        {
            volatile uint64_t addr = currentNode->addr;        // Get the address of the cache line
            CYCLES time = measure_one_block_access_time(addr); // Measure access time
            accessTimes[set][cacheLineAccessIter] = time;      // Store access time
            currentNode = currentNode->next;                   // Move to the next cache line
            cacheLineAccessIter++;
        }
    }

    // Analyze access times to determine which set was accessed
    for (int set = 0; set < L2_SETS_COMM; set++)
    {
        int misses = 0;

        // Count cache misses in the current set by comparing access times to a threshold
        for (int i = 0; i < cacheLineAccess; i++)
        {
            if (accessTimes[set][i] >= 46 && accessTimes[set][i] < 100) // Miss threshold
            {
                misses++;
            }
        }

        // If all cache lines in a set have misses, consider it as a detected set
        if (misses == cacheLineAccess)
        {
            missesArray[set]++; // Increment miss count for the current set

            // If a set has been detected BIT_THRESHOLD times, return it as the result
            // Else the receiver loop will be in busy looping
            if (missesArray[set] > BIT_THRESHOLD)
            {
                returnSet = set;
                bitFlag = 1;
            }
        }
    }

    // Return the detected set (or -1 if none is detected)
    if (returnSet == L2_SETS_COMM)
        // return 0 if bit is L2_SETS_COMM
        return 0;
    return returnSet;
}

// Configures the receiver setup
// Allocates memory for eviction sets and configures prime, probe, and access time waits
void receiver_config(volatile struct setup *setup)
{
    // Allocate a large buffer using mmap and trigger page allocation
    void *buf = mmap(NULL, BUFF_SIZE, PROT_READ | PROT_WRITE,
                     MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE | MAP_HUGETLB, -1, 0);
    if (buf == MAP_FAILED)
    {
        perror("mmap() error");
        exit(EXIT_FAILURE);
    }
    *((volatile char *)buf) = 1; // Trigger the page allocation by writing to the buffer

    setup->eviction_buffer = buf; // Store the buffer address in the setup structure

    // Create eviction sets for all sets in the cache
    create_eviction_sets(setup);
    setup->primeTimeWait = PRIME_TIME;   // Set the prime time wait
    setup->probeTimeWait = PROBE_TIME;   // Set the probe time wait
    setup->accessTimeWait = ACCESS_TIME; // Set the access time wait
}

int main()
{
    printf("Receiver now listening.\n");

    // Allocate memory for the setup structure
    volatile struct setup *setupStruct = (struct setup *)malloc(sizeof(struct setup));

    // Configure the receiver
    receiver_config(setupStruct);

    int missesArray[L2_SETS_COMM] = {0}; // Array to track misses for each set

    // Continuously listen for bits being sent by the sender
    while (1)
    {
        int detected_set = receive_bit(setupStruct, missesArray); // Receive a bit by detecting a set
        if (detected_set != -1)
            printf("Detected set accessed: %d\n", detected_set); // Output the detected set
    }

    printf("Receiver finished.\n");
    return 0;
}
