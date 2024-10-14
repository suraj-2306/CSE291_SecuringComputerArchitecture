
#include "util.h"
// mman library to be used for hugepage allocations (e.g. mmap or posix_memalign only)
#include <sys/mman.h>
int receive_bit(struct setup *setup)
{
    struct Node *currentNode;
    currentNode = setup->eviction_ll;

    for(volatile int setNum;setNum<2;setNum++){
        clock_t start_t, curr_t, end_t;
    volatile int totalTime = 0;
    volatile int cacheLineAccess = 5, cacheLineAccessIter = 0;

    // prime stage
    start_t = clock();
    while (currentNode != NULL && clock() - start_t < setup->primeTimeWait && cacheLineAccessIter < cacheLineAccess)
    {
        cacheLineAccessIter++;
        volatile uint64_t addr1 = currentNode->addr;
        one_block_access(addr1);
        currentNode = currentNode->next;
    };

    // acccess interval
    start_t = clock();
    while (clock() - start_t < (setup->accessTimeWait))
    {
    };

    // probe stage
    int misses = 0, hits = 0;
    volatile uint64_t accessTime[4];

    cacheLineAccess = 4;
    cacheLineAccessIter = 0;
    start_t = clock();
    while (currentNode != NULL && (clock() - start_t < setup->probeTimeWait) && cacheLineAccessIter < cacheLineAccess)
    {
        cacheLineAccessIter++;
        volatile uint64_t addr1 = currentNode->addr;
        accessTime[cacheLineAccessIter - 1] = measure_one_block_access_time(addr1);

        if (accessTime[cacheLineAccessIter - 1] > 42)
            misses++;
        else
            hits++;
        currentNode = currentNode->next;
        // printf("accessTime is %d", accessTime);
    }

    if (hits == 1 && misses == 3)
        return 0;
    else if (hits == 3 && misses == 1)
        return 1;
    else if (hits == 4)
        return 5;
    else if (misses == 4)
        return 4;
    else if (hits + misses < 4)
        return 2;
    else if (hits == 2 && misses == 2)
        return 6;
    else
        return 3;

    }

    
}
int receiver_config(struct setup *setup)
{

    void *buf = mmap(NULL, BUFF_SIZE, PROT_READ | PROT_WRITE, MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE | MAP_HUGETLB, -1, 0);

    if (buf == (void *)-1)
    {
        perror("mmap() error\n");
        exit(EXIT_FAILURE);
    }
    // The first access to a page triggers overhead associated with
    // page allocation, TLB insertion, etc.
    // Thus, we use a dummy write here to trigger page allocation
    // so later access will not suffer from such overhead.
    *((char *)buf) = 1; // dummy write to trigger page allocation

    setup->eviction_buffer = buf;

    setup->primeTimeWait = 150;
    setup->probeTimeWait = 150;
    setup->accessTimeWait = 70;
    setup->eviction_ll_0 = create_eviction_set(setup, 0);
    setup->eviction_ll_1 = create_eviction_set(setup, 1);
}
int main(int argc, char **argv)
{
    // [Bonus] TODO: Put your covert channel setup code here

    // printf("Please press enter.\n");

    // char text_buf[2];
    // fgets(text_buf, sizeof(text_buf), stdin);

    printf("Receiver now listening.\n");

    // struct setup *setup;
    struct setup *setupStruct = (struct setup *)malloc(sizeof(struct setup));
    receiver_config(setupStruct);

    // send_bit(setup, 0);

    bool listening = true;
    int bit;
    while (listening)
    {
        bit = receive_bit(setupStruct);
        // printf("received bit %d\n", bit);
        // if (bit == 1 || bit == 0)
        printf("Received bit %d\n", bit);
        // listening = false;

        // [Bonus] TODO: Put your covert channel code here
    }

    printf("Receiver finished.\n");

    return 0;
}
