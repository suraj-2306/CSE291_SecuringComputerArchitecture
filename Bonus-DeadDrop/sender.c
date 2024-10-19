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

    // Access stage: Access the cache lines in the selected eviction set
    volatile clock_t start_t = rdtsc();
    while ((rdtsc() - start_t) < setup->accessTimeWait)
    {
        cacheLineAccessIter = 0;
        while (currentNode != NULL &&
               cacheLineAccessIter < cacheLineAccess)
        {
            // printf("iter %d\n", cacheLineAccessIter);

            volatile uint64_t *addr = (volatile uint64_t *)currentNode->addr;

            for (volatile int n = 0; n < 2; n++)
            {
                one_block_access((volatile uint64_t)addr);
            }
            // printf("set no %d, cachelineIter %d\n", bit, cacheLineAccessIter);
            cacheLineAccessIter++;
            currentNode = currentNode->next;
        }
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

    uint8_t bit_to_send = 12; // Example bit (set number) to send

    while (1)
    {
        // Prompt the user for input
        printf("Enter a number between 0 and %d (inclusive): ", L2_SETS_DEFINE);
        int input;
        int result = scanf("%d", &input);

        // Check if the input is valid and within the range
        if (result == 1 && input >= 0 && input <= L2_SETS_DEFINE)
        {
            bit_to_send = (uint8_t)input;       // Cast the valid input to uint8_t
            send_bit(setupStruct, bit_to_send); // Send the bit
            while (1)
                ;
        }
        else
        {
            printf("Invalid input! Please enter a number between 0 and %d.\n", L2_SETS_DEFINE);
            // Clear the input buffer to avoid infinite loop on invalid input
            while (getchar() != '\n')
                ;
        }
    }

    printf("Sender finished.\n");
    return 0;
}