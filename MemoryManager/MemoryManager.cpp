#include <cstring>
#include <unistd.h>
#include <cstdint>
#include <functional>
#include <cmath>
#include <iostream>
#include "MemoryManager.h"

#define MAX_WORDS 65536
#define MAX_WORD_SIZE_IN_BYTES 16
#define INVALID_WORD_SIZE "number of words for size cannot be zero. ERROR\n"

enum Status { Free, Used, Boundary };

/* Constructor that sets native word size (for alignment)
 * and default allocator function for finding a memory 
 * hole. */
MemoryManager::MemoryManager(unsigned wordSize, std::function<int(int, void*)> allocator) {
	if (wordSize <= MAX_WORD_SIZE_IN_BYTES && wordSize > 0)
		word_size = wordSize;
	else if (wordSize == 0)
		// What do I do when word is 0 bytes?
		std::cout << INVALID_WORD_SIZE << std::endl;
	else
		word_size = MAX_WORD_SIZE_IN_BYTES;
	
	this->allocator = allocator;
}

/* Releases all memory allocated by this object without
 * leaking memory. */
MemoryManager::~MemoryManager() {
	
}

/* Instantiates a new block of memory of requested size,
 * no larger than 65536. Hint: you may use new char[...] */
void MemoryManager::initialize(size_t sizeInWords) {
	// Check if sizeInWords is less than the limit of 65536
	if (sizeInWords < MAX_WORDS) {
		memory_blocks = word_size * sizeInWords;
		unsigned char *memory_addr = new unsigned char[memory_blocks];
		unsigned char *memory_bitmap = new unsigned char[sizeInWords];
		
		for (size_t i = 0; i < sizeInWords; i++)
			memory_bitmap[i] = Free;
	}
}

/* Releases memory block acquired during initialization. */
void MemoryManager::shutdown() {
	
}

/* Allocates a block ofmemory. If no memory is available or
 * size is invalid, return nullptr. */
void* MemoryManager::allocate(size_t sizeInBytes) {
	if (sizeInBytes == 0)
		return nullptr;
	
	size_t num_blocks_required = ceil((double)sizeInBytes / (double) word_size);
	size_t location = 0;
	
	if (allocator(sizeInBytes, &memory_bitmap) > 0) {
		
	}
}

/* Frees the memory block within the memory manager so that
 * it can be reused. */
void MemoryManager::free(void *address) {
	
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
	
}

/* Returns a bit-stream of bits representing whether words
 * are used (1) or free (0).The first two bytes are the 
 * size of the bitmap(little-Endian). The rest is the
 * bitmap, word-wise. */
void* MemoryManager::getBitmap() {
	
}

/* Returns the word size used for alignment. */
unsigned MemoryManager::getWordSize() {
	return this->word_size;
}

/* Returns the byte-wise memory address of the beginning
 * of the memory block. */
void* MemoryManager::getMemoryStart() {
	
}

/* Returns the byte limit of the current memory block. */
unsigned MemoryManager::getMemoryLimit() {
	
}

/** Memory Allocation Algorithms ****************/
/* Returns word offset of hole selected by the best fit
 * memory allocation algorithm, and -1 if there is no fit. */
int MemoryManager::bestFit(int size, void *list) {
	
}

/* Returns word offset of hole selected by the worst fit
 * memory allocation algorithm, and -1 if there is no fit. */
int MemoryManager::worstFit(int size, void *list) {
	
}

/* void* operator new(size_t size) {
	
}

void operator new(void* ptr) {
	
} */