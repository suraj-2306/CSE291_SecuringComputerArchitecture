
#include "util.h"
// mman library to be used for hugepage allocations (e.g. mmap or posix_memalign only)
#include <sys/mman.h>

// [Bonus] TODO: define your own buffer size

// #define BUFF_SIZE TODO

void send_bit(struct setup *setup, uint8_t bit)
{
    struct Node *currentNode;

    volatile int cacheLineAccess;
    if (bit)
        currentNode = setup->eviction_ll_1;
    else
        currentNode = setup->eviction_ll_0;

    cacheLineAccess = 6;

    // prime wait
    clock_t start_t = clock();
    while (clock() - start_t < (setup->primeTimeWait))
    {
    };

    // access cache
    start_t = clock();
    volatile int cacheLineAccessIter = 0;
    while (currentNode != NULL && clock() - start_t < (setup->accessTimeWait) && cacheLineAccessIter < cacheLineAccess)
    {
        cacheLineAccessIter++;
        volatile uint64_t addr1 = currentNode->addr;
        one_block_access(addr1);
        currentNode = currentNode->next;
    }

    // probe wait
    start_t = clock();

    while (clock() - start_t < (setup->primeTimeWait))
    {
    };
}

int sender_config(struct setup *setup)
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

// printf("created eviction set\n\n");

int main(int argc, char **argv)
{
    // Allocate a buffer using huge page
    // See the handout for details about hugepage management

    // [Bonus] TODO:
    // Put your covert channel setup code here

    // struct setup *setup;
    struct setup *setupStruct = (struct setup *)malloc(sizeof(struct setup));
    sender_config(setupStruct);

    // send_bit(setupStruct, 0);

    printf("Please type a message.\n");

    bool sending = true;
    while (sending)
    {
        // char text_buf[128];
        // send_bit(setupStruct, 0);
        send_bit(setupStruct, 1);
        send_bit(setupStruct, 1);
        send_bit(setupStruct, 1);
        send_bit(setupStruct, 1);
        send_bit(setupStruct, 1);
        // send_bit(setupStruct, 0);
        // fgets(text_buf, sizeof(text_buf), stdin);

        // [Bonus] TODO:
        // Put your covert channel code here
    }

    printf("Sender finished.\n");
    return 0;
}
