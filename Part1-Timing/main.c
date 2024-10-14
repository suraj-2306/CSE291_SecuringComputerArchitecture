#include "utility.h"

#ifndef VISUAL
#define PRINT_FUNC print_results_plaintext
#else
#define PRINT_FUNC print_results_for_visualization
#endif

// #define LINE_SIZE 64
// // // [1.2] TODO: Uncomment the following lines and fill in the correct size
#define LINE_SIZE 64              // Cache line size in bytes
#define L1_SIZE (32 * 1024)       // 32 KB for L1 cache size
#define L2_SIZE (256 * 1024)      // 256 KB for L2 cache size
#define L3_SIZE (8 * 1024 * 1024) // 8 MB for L3 cache size
#define BUFF_SIZE (4 * L3_SIZE)   // Large eviction buffer to ensure L3 eviction

#define L1_ASSOCIATIVITY 8
#define L2_ASSOCIATIVITY 4
#define L3_ASSOCIATIVITY 16

#define L1_NUM_SETS (L1_SIZE / (L1_ASSOCIATIVITY * LINE_SIZE))
#define L2_NUM_SETS (L2_SIZE / (L2_ASSOCIATIVITY * LINE_SIZE))
#define L3_NUM_SETS (L3_SIZE / (L3_ASSOCIATIVITY * LINE_SIZE))

int main(int ac, char **av)
{

    // create 4 arrays to store the latency numbers
    // the arrays are initialized to 0
    uint64_t dram_latency[SAMPLES] = {0};
    uint64_t l1_latency[SAMPLES] = {0};
    uint64_t l2_latency[SAMPLES] = {0};
    uint64_t l3_latency[SAMPLES] = {0};

    // A temporary variable we can use to load addresses
    uint8_t tmp;

    // Allocate a buffer of LINE_SIZE Bytes
    // The volatile keyword tells the compiler to not put this variable into a
    // register -- it should always try to be loaded from memory / cache.
    volatile uint8_t *target_buffer = (uint8_t *)malloc(LINE_SIZE);

    if (NULL == target_buffer)
    {
        perror("Unable to malloc");
        return EXIT_FAILURE;
    }

    // [1.2] TODO: Uncomment the following line to allocate a buffer of a size
    // of your chosing. This will help you measure the latencies at L2 and L3.
    volatile uint8_t *eviction_buffer = (uint8_t *)malloc(BUFF_SIZE);

    // Example: Measure L1 access latency, store results in l1_latency array
    for (int i = 0; i < SAMPLES; i++)
    {
        // Step 1: bring the target cache line into L1 by simply accessing
        //         the line
        tmp = target_buffer[0];
        lfence();

        // Step 2: measure the access latency
        l1_latency[i] = measure_one_block_access_time((uint64_t)target_buffer);
    }

    // ======
    // [1.2] TODO: Measure DRAM Latency, store results in dram_latency array
    // ======
    for (int i = 0; i < SAMPLES; i++)
    {
        tmp = target_buffer[0];
        clflush((void *)target_buffer);
        // Step 2: measure the access latency
        dram_latency[i] = measure_one_block_access_time((uint64_t)target_buffer);
    }

    // ======
    // [1.2] TODO: Measure L2 Latency, store results in l2_latency array
    // ======
    for (int i = 0; i < SAMPLES; i++)
    {
        // Step 1: bring the target cache line into L1 by simply accessing
        //         the line
        tmp = target_buffer[0];
        int n = 4;
        while (n--)
        {
            for (int set = 0; set < L1_NUM_SETS; set++)
            {
                for (int way = 0; way < L1_ASSOCIATIVITY + 1; way++)

                {
                    for (int block = 0; block < LINE_SIZE; block++)
                    {
                        one_block_access((uint64_t)eviction_buffer + LINE_SIZE * (way + L1_ASSOCIATIVITY * set) + block);
                        lfence();
                    }
                }
            }
        }
        // Step 2: measure the access latency
        l2_latency[i] = measure_one_block_access_time((uint64_t)target_buffer);
    }
    // ======
    // [1.2] TODO: Measure L3 Latency, store results in l3_latency array
    // ======
    //
    for (int i = 0; i < SAMPLES; i++)
    {
        // Step 1: bring the target cache line into L1 by simply accessing
        //         the line
        tmp = target_buffer[0];
        int n = 4;
        while (n--)
        {
            for (int set = 0; set < L2_NUM_SETS; set++)
            {
                for (int way = 0; way < L2_ASSOCIATIVITY + 1; way++)
                {
                    for (int block = 0; block < LINE_SIZE; block++)
                    {
                        one_block_access((uint64_t)eviction_buffer + LINE_SIZE * (way + L2_ASSOCIATIVITY * set) + block);
                        lfence();
                    }
                }
            }
        }
        // Step 2: measure the access latency
        l3_latency[i] = measure_one_block_access_time((uint64_t)target_buffer);
    }

    // Print the results to the screen
    // When compile to main and used by `make run`,
    // it uses print_results_plaintext
    // When compile to main-visual and used by `run.py`,
    // it uses print_results_for_visualization
    PRINT_FUNC(dram_latency, l1_latency, l2_latency, l3_latency);

    free((uint8_t *)target_buffer);

    // [1.2] TODO: Uncomment this line once you uncomment the eviction_buffer
    //             creation line
    free((uint8_t *)eviction_buffer);
    return 0;
}
