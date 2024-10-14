
#include "util.h"
// mman library to be used for hugepage allocations (e.g. mmap or posix_memalign only)
#include <sys/mman.h>
int receive_bit(struct setup *setup)
{
    struct Node *currentNode;
    // currentNode = setup->eviction_ll;

    int misses[2] = {0, 0}, hits[2] = {0, 0};
    for (volatile int setNum = 0; setNum < 2; setNum++)
    {
        if (setNum == 0)
            currentNode = setup->eviction_ll_0;
        else
            currentNode = setup->eviction_ll_1;

        clock_t start_t, curr_t, end_t;
        volatile int totalTime = 0;
        volatile int cacheLineAccess = 6, cacheLineAccessIter = 0;

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
        if (setNum == 0)
            currentNode = setup->eviction_ll_0;
        else
            currentNode = setup->eviction_ll_1;
        volatile uint64_t accessTime[4];

        cacheLineAccess = 4;
        cacheLineAccessIter = 0;
        start_t = clock();
        while (currentNode != NULL && clock() - start_t < setup->probeTimeWait && cacheLineAccessIter < cacheLineAccess)
        {
            cacheLineAccessIter++;
            volatile uint64_t addr1 = currentNode->addr;
            accessTime[cacheLineAccessIter - 1] = measure_one_block_access_time(addr1);

            if (accessTime[cacheLineAccessIter - 1] > 37)
                misses[setNum]++;
            else
                hits[setNum]++;
            currentNode = currentNode->next;
            // printf("accessTime is %ld", accessTime[cacheLineAccessIter]);
        }
    }

    if (misses[1] >= 4 && hits[1] <= 0)
        return 1;
    else if (misses[0] >= 4 && hits[0] <= 0)
        return 0;
    else if (misses[1] == 0 && misses[0] == 0)
        return 3;
    else if (misses[1] <= 2 && misses[0] <= 2)
        return 2;
    else
        return 4;
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
    setup->primeTimeWait = 1500;
    setup->probeTimeWait = 1500;
    setup->accessTimeWait = 1200;
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
