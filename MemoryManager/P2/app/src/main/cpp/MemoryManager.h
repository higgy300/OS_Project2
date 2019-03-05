//
// Created by juanh on 2/20/2019.
//

#ifndef OS_P2_MEMORYMANAGER_H
#define OS_P2_MEMORYMANAGER_H
#include <functional>
//#include <unistd.h>
#include <stdint.h>
#include <vector>
/** Memory Manger Class *************************/
class MemoryManager {
public:
    /* Constructor that sets native word size (for alignment)
     * and default allocator function for finding a memory
     * hole. */
    MemoryManager(unsigned wordSize, std::function<int(int, void*)> allocator);
    /* Releases all memory allocated by this object without
     * leaking memory. */
    ~MemoryManager();

    /* Instantiates a new block of memory of requested size,
     * no larger than 65536. Hint: you may use new char[...] */
    void initialize(size_t sizeInWords);

    /* Releases memory block acquired during initialization. */
    void shutdown();

    /* Allocates a block of memory. If no memory is available or
     * size is invalid, return nullptr. */
    void *allocate(size_t sizeInBytes);

    /* Frees the memory block within the memory manager so that
     * it can be reused. */
    void free(void *address);

    /* Changes the allocation algorithm to identifying the
     * memory hole to use  for allocation. */
    void setAllocator(std::function<int(int, void*)> allocator);

    /* Uses standard POSIX calls to writehole list of
     * filename as text, returning -1 on error and 0 if
       successful.

       Format: "[START, LENGTH] - [START, LENGTH] ... "
       example:  [0,10] - [12,2] - [20,6]             */
    int dumpMemoryMap(char *filename);

    /* Returns a byte-stream of information (in binary) about
     * holes for use by the allocator function(little-Endian). */
    void *getList();

    /* Returns a bit-stream of bits representing whether words
     * are used (1) or free (0).The first two bytes are the
     * size of the bitmap(little-Endian). The rest is the
     * bitmap, word-wise. */
    void *getBitmap();

    /* Returns the word size used for alignment. */
    unsigned getWordSize();

    /* Returns the byte-wise memory address of the beginning
     * of the memory block. */
    void *getMemoryStart();

    /* Returns the byte limit of the current memory block. */
    unsigned getMemoryLimit();

private:
    std::function<int(int, void*)> allocator;
    uint16_t word_size;
    uint16_t bitmap_size;
    uint16_t num_of_words;
    uint16_t size_in_words;
    uint32_t memory_byte_blocks;
    uint8_t *memory_addr;
    uint8_t *memory_bitmap;
    std::vector<uint16_t> hole_list;
    std::vector<uint16_t> proc_list;

    void updateBitmap(int start, int last, bool allocate);
    void updateHoleList();
    void updateProcList(int i, int i1, bool b);

};

/*********** Memory Allocation Algorithms ************************/
/* Returns word offset of hole selected by the best fit
 * memory allocation algorithm, and -1 if there is no fit. */
int bestFit(int size, void *list);

/* Returns word offset of hole selected by the worst fit
 * memory allocation algorithm, and -1 if there is no fit. */
int worstFit(int size, void *list);
#endif //OS_P2_MEMORYMANAGER_H
