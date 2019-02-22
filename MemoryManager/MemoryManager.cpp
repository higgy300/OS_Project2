#include <utility>

#include <cstring>
//#include <unistd.h>
#include <cstdint>
#include <functional>
#include <vector>
#include <cmath>
#include <iostream>
#include "MemoryManager.h"

#define MAX_WORDS 65536
#define MAX_WORD_SIZE_IN_BYTES 16
#define INVALID_WORD_SIZE "number of words for size cannot be zero. ERROR\n"
#define USED 1
#define FREE 0

/* Constructor that sets native word size (for alignment)
 * and default allocator function for finding a memory
 * hole. */
MemoryManager::MemoryManager(unsigned wordSize, std::function<int(int, void*)> allocator) {
    if (wordSize <= MAX_WORD_SIZE_IN_BYTES && wordSize > 0)
        word_size = static_cast<uint16_t>(wordSize);
    else if (wordSize == 0)
        // TODO:********What do I do when word is 0 bytes?
        std::cout << INVALID_WORD_SIZE << std::endl;
    else
        word_size = MAX_WORD_SIZE_IN_BYTES;

    this->allocator = allocator;
}

/* Releases all memory allocated by this object withoutc
 * leaking memory. */
MemoryManager::~MemoryManager() {
    shutdown();
    delete this;
}

/* Instantiates a new block of memory of requested size,
 * no larger than 65536. Hint: you may use new char[...] */
void MemoryManager::initialize(size_t sizeInWords) {
    // Check if sizeInWords is less than the limit of 65536
    if (sizeInWords < MAX_WORDS) {
        //num_of_words = static_cast<uint16_t>(ceil((double)sizeInWords / (double)word_size));
        num_of_byte_blocks = static_cast<uint32_t>(word_size * sizeInWords);
        uint8_t *memAddr = new uint8_t[num_of_byte_blocks];
        memory_addr = memAddr;
        uint16_t _bitmap_size = static_cast<uint16_t>(sizeInWords + 2);
        bitmap_size = _bitmap_size;
        uint8_t *bitmap = new uint8_t[_bitmap_size];
        memory_bitmap = bitmap;

        // Store stream length in first two bytes of bitmap
        uint8_t low_byte = (uint8_t)_bitmap_size;
        uint8_t high_byte = (uint8_t)(_bitmap_size >> 8);
        memory_bitmap[0] = high_byte;
        memory_bitmap[1] = low_byte;

        //std::cout << "LOW: "<< (unsigned) memory_bitmap[0] <<
        // "\nHIGH: " << (unsigned) memory_bitmap[1] << std::endl;


        // Mark all words in bitmap as free
        for (size_t i = 2; i < _bitmap_size; i++)
            memory_bitmap[i] = FREE;

        /* Initialize list (4 = number of bytes), (0 = initial offset)*/
        hole_list.push_back(4);
        hole_list.push_back(0);
        hole_list.push_back(static_cast<unsigned short &&>(sizeInWords));

    } //else
    //TODO:*************throw exception when words is 0
}

/* Releases memory block acquired during initialization. */
void MemoryManager::shutdown() {
    delete []memory_addr;
    delete []memory_bitmap;
    std::vector<uint16_t >().swap(hole_list);
}

/* Allocates a block of memory. If no memory is available or
 * size is invalid, return nullptr. */
void* MemoryManager::allocate(size_t sizeInBytes) {
    // Check for valid parameter input
    if (sizeInBytes == 0)
        return nullptr;

    /* Calculate how many bytes are required to allocate
     * from the address space */
    uint16_t num_words_required = static_cast<uint16_t>(ceil((double)sizeInBytes / (double) word_size));
    size_t num_of_blocks_needed = num_words_required * word_size;

    /* Call the allocation algorithm to obtain the offset of
     * of memory address. */
    int offset = allocator(num_words_required, (void*)&hole_list);
    int offset_in_single_bytes = offset * word_size;

    // These are the addresses from memory address
    uint8_t* starting_addr = memory_addr + offset_in_single_bytes;
    uint8_t* end_addr = starting_addr + num_of_blocks_needed;

    /* Check if allocation algorithm did not fail. Else, return
     * a null pointer. */
    if (offset >= 0) {
        /* Calculate bitmap index to determine where the new
         * block is marked as USED and the last index to stop
         * marking. */
        int start = 2 + offset;
        int last = static_cast<int>(start + num_words_required);

        // Mark address space in bitmap as USED
        updateBitmap(start, last, true);

        /* Check if hole list is empty. If empty, return null pointer.
         * Otherwise, update hole list */
        if (hole_list.size() == 1)
            return nullptr;
        else
            updateHoleList();

        // Return starting address where new block of memory is allocated
        return (void*)starting_addr;
    } else
        return nullptr;
}

/* Frees the memory block within the memory manager so that
 * it can be reused. */
void MemoryManager::free(void *address) {
    uint8_t *free_addr = reinterpret_cast<uint8_t*>(address);
    uint16_t offset = static_cast<uint16_t>((free_addr - memory_addr) / word_size);

    int start = offset + 2, last = start + 1;

    if (memory_bitmap[offset + 2] == FREE) {
        return;
    } else {
        updateBitmap(start, last, false);
    }

}

/* Changes the allocation algorithm to identifying the
 * memory hole to use  for allocation. */
void MemoryManager::setAllocator(std::function<int(int, void*)> allocator) {
    this->allocator = allocator;
}

/* Uses standard POSIX calls to writehole list of
 * filename as text, returning -1 on error and 0 if
   successful.

   Format: "[START, LENGTH] - [START, LENGTH] ... "
   example:  [0,10] - [12,2] - [20,6]             */
int MemoryManager::dumpMemoryMap(char *filename) {

}


/* Returns a byte-stream of information (in binary) about
 * holes for use by the allocator function(little-Endian). */
void* MemoryManager::getList() {
    return &hole_list;
}

/* Returns a bit-stream of bits representing whether words
 * are used (1) or free (0).The first two bytes are the
 * size of the bitmap(little-Endian). The rest is the
 * bitmap, word-wise. */
void* MemoryManager::getBitmap() {
    return &memory_bitmap;
}

/* Returns the word size used for alignment. */
unsigned MemoryManager::getWordSize() {
    return word_size;
}

/* Returns the byte-wise memory address of the beginning
 * of the memory block. */
void* MemoryManager::getMemoryStart() {
    return &memory_addr;
}

/* Returns the byte limit of the current memory block. */
unsigned MemoryManager::getMemoryLimit() {
    return num_of_byte_blocks;
}

void MemoryManager::updateHoleList() {
    uint16_t temp_offset = 0, temp_length = 0;
    std::vector<uint16_t> temp_vect;



    for (int i = 2; i < bitmap_size; i++) {
        if (memory_bitmap[i] == 0) {
            temp_offset = (uint16_t )i;
            ++temp_length;
            while ((i + 1 < bitmap_size) || (memory_bitmap[i + 1] == 1)) {
                ++temp_length;
                ++i;
            }
            temp_vect.push_back(temp_offset);
            temp_vect.push_back(temp_length);
        }
    }

    uint16_t temp_size = static_cast<uint16_t>(temp_vect.size());
    uint8_t low = (uint8_t)temp_size;
    uint8_t high = (uint8_t)(temp_size >> 8);
    temp_vect.insert(temp_vect.begin(),low);
    temp_vect.insert(temp_vect.begin(),high);
    hole_list.swap(temp_vect);
}

void MemoryManager::updateBitmap(int start, int last, bool allocate) {
    bool temp;
    if (allocate)
        temp = USED;
    else
        temp = FREE;

    for (int i = start; i < last; i++)
        memory_bitmap[i] = temp;
}

/** Memory Allocation Algorithms ****************/
/* Returns word offset of hole selected by the best fit
 * memory allocation algorithm, and -1 if there is no fit. */
int bestFit(int size, void *list) {
    int offset = -1, smallest_length = 2000000000;
    std::vector<uint16_t> _list = *reinterpret_cast<std::vector<uint16_t>*>(list);

    /* Check all holes to find smallest hole we can use to fit the
     * desired block of memory */
    for (int i = 1; i < _list.size(); i += 2) {
        /* Does the block we are trying to allocate fit in the current
         * hole ? */
        if (size <= _list[i + 1]) {
            /* It does fit! Is the hole smaller then the smallest hole we
             * have seen so far? */
            if (_list[i + 1] <= smallest_length) {
                /* It is smaller! Let's mark it as smallest hole that can
                 * fit our desired block of memory */
                smallest_length = _list[i + 1];
                offset = _list[i];
            }
        }
    }

    return offset;
}

/* Returns word offset of hole selected by the worst fit
 * memory allocation algorithm, and -1 if there is no fit. */
int worstFit(int size, void *list) {
    int offset = -1, biggest_length = 0;
    std::vector<uint16_t> _list = *reinterpret_cast<std::vector<uint16_t>*>(list);

    /* Check all holes to find smallest hole we can use to fit the
     * desired block of memory */
    for (int i = 1; i < _list.size(); i += 2) {
        /* Does the block we are trying to allocate fit in the current
         * hole ? */
        if (size <= _list[i + 1]) {
            /* It does fit! Is the hole bigger then the biggest hole we
             * have seen so far? */
            if (_list[i + 1] >= biggest_length) {
                /* It is smaller! Let's mark it as smallest hole that can
                 * fit our desired block of memory */
                biggest_length = _list[i + 1];
                offset = _list[i];
            }
        }
    }

    return offset;
}