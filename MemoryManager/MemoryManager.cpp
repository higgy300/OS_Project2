#include <utility>
#include <cstring>
#include <unistd.h>
#include <cstdint>
#include <functional>
#include <vector>
#include <cmath>
#include <iostream>
#include <cstdlib>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/syscall.h>
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
    /* Check if wordSize parameter is within a 16 byte limit because
       a word size cannot be greater than 16 bytes. That's 64 bits */
    if (wordSize <= MAX_WORD_SIZE_IN_BYTES && wordSize > 0)
        word_size = static_cast<uint16_t>(wordSize);
    else if (wordSize == 0) // If the wordSize is zero
        word_size = 2;
    else // If the word size is greater than 16 bytes, use max of 16 bytes by default
        word_size = MAX_WORD_SIZE_IN_BYTES;

    // Make a copy of the allocator function (either bestFit() or worstFit())
    this->allocator = allocator;
}

/* Releases all memory allocated by this object withoutc
 * leaking memory. */
MemoryManager::~MemoryManager() {
    // Deallocate every dynamic variable and vector
    shutdown();
}

/* Instantiates a new block of memory of requested size,
 * no larger than 65536. Hint: you may use new char[...] */
void MemoryManager::initialize(size_t sizeInWords) {
    // Check if sizeInWords is less than the limit of 65536
	if (!initialized) {
		if (sizeInWords < MAX_WORDS) {
			initialized = true;
			//num_of_words = static_cast<uint16_t>(ceil((double)sizeInWords / (double)word_size));
			// How many blocks of one byte are needed to allocate this space?
			memory_byte_blocks = static_cast<uint32_t>(word_size * sizeInWords);
			
			std::vector<uint16_t> temp;
			
			// Allocate the space
			auto *memAddr = new uint8_t[memory_byte_blocks];

			// Initialize memory spaces to zero

			// Store the starting address to the class object
			memory_addr = memAddr;

			/* Calculate how many blocks of 2 bytes are required to keep track of
			   each word to be allocated in the giant block of memory and store it
			   to the class object to be able to iterate later */
			auto _bitmap_size = static_cast<uint16_t>(sizeInWords + 2);
			bitmap_size = _bitmap_size;

			// allocate the bitmap space
			auto *bitmap = new uint8_t[_bitmap_size];
			// Initialize everything in bitmap to 0
			for (int i = 0; i < _bitmap_size; i++)
				bitmap[i] = 0;
			// Save the initial bitmap address to the class object
			memory_bitmap = bitmap;

			// Store stream length in first two bytes of bitmap
			auto low_byte = (uint8_t)_bitmap_size;
			auto high_byte = (uint8_t)(_bitmap_size >> 8);
			memory_bitmap[0] = high_byte;
			memory_bitmap[1] = low_byte;

			// Mark all words in bitmap as free
			updateBitmap(2, _bitmap_size, false);

			/* Initialize list (4 = number of bytes), (0 = initial offset)
			 * sizeInWords is the length of the hole in words */
			temp.push_back(4);
			temp.push_back(0);
			temp.push_back(static_cast<unsigned short &&>(sizeInWords));
			hole_list.swap(temp);
		}
	}
     //else
    //TODO:*************throw exception when sizeInWords is 0
}

/* Releases memory block acquired during initialization. */
void MemoryManager::shutdown() {
    /* To prevent this function from being called twice in a row.
       the variable initialized will only change to true after initialize
       is called and it will only change to false after shutdown is called.
       This maintains a one to one ratio to prevent segmentation faults. */
    if (initialized) {
        initialized = false;
        delete[]memory_addr;
        delete[]memory_bitmap;
        std::vector<uint16_t>().swap(hole_list);
        std::vector<uint16_t>().swap(proc_list);
    }
}

/* Allocates a block of memory. If no memory is available or
 * size is invalid, return nullptr. */
void* MemoryManager::allocate(size_t sizeInBytes) {
    // Check for valid parameter input
    if (sizeInBytes == 0)
        return nullptr;

    /* Calculate how many bytes are required to be able to allocate
     * from the giant block of memory */
    auto num_words_required = static_cast<uint16_t>(ceil((double)sizeInBytes / (double) word_size));

    /* Call the allocation algorithm currently assigned to the class object
     * to obtain the offset of starting memory address. Offset is the number
     * it takes from the first address of the giant block of space to the
     * location of the first word we are about to use */
    int offset = allocator(num_words_required, (void*)&hole_list);
    int offset_in_single_bytes = offset * word_size;

    /* These are the addresses from the new block of memory we are about to
       allocate */
    uint8_t* starting_addr = &memory_addr[offset_in_single_bytes];

    /* Check if allocation algorithm did not fail. Else, return
     * a null pointer. */
    if (offset >= 0) {
        /* Calculate bitmap index to determine where the new
         * block is marked as USED and the last index to stop
         * marking. */
        int start = 2 + offset;
        int last = start + num_words_required;

        // Mark address space in bitmap as USED
        updateBitmap(start, last, true);

        // Update list of existing processes
        updateProcList(offset, num_words_required, false);

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
    // Cast void pointer parameter into a memory address of address space so we can do stuff with this address
    auto *free_addr = reinterpret_cast<uint8_t*>(address);
    //uint32_t offset = 0;

    /*for (auto itr = &memory_addr; itr != free_addr; itr++) {
        ++offset;
    }*/
    auto offset = static_cast<uint16_t>((free_addr - memory_addr) / word_size);

    if (memory_bitmap[offset + 2] == FREE)
        return;
    else {
        /* Find the length of the block that was originally allocated to do a complete
       deallocation of the block and not just a single word deallocation */
        uint16_t length = 0;

        for (uint16_t i = 0; i < proc_list.size(); i += 2) {
            if (proc_list[i] == offset) {
                length = proc_list[i + 1];
                break;
            }
        }

        // Mark the block of memory as free
        updateBitmap(offset + 2, offset + 2 + length, false);

        // Update hole list
        updateHoleList();

        // Update process list
        updateProcList(offset, 0, true);
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
	int fd1 = open(filename, O_WRONLY | O_CREAT, S_IRWXU);
	if (fd1 == -1) {
        perror("File not opened.\n");
        return -1;
    } else {
		std::string txt = "[" + std::to_string(hole_list[1]) + 
			", " + std::to_string(hole_list[2]);
		
		for (int i = 3; i < hole_list.size() - 1; i++) {
			txt += "] - [" + std::to_string(hole_list[i]) + ", " +
				std::to_string(hole_list[i + 1]);
		}
		txt += "]";
		char buf[txt.length()];
		
		for (int i = 0; i < txt.length() + 1; i++) {
			buf[i] = txt[i];
		}
		
		write(fd1, buf, strlen(buf));
	}
	
	close(fd1);
    return 0;
}


/* Returns a byte-stream of information (in binary) about
 * holes for use by the allocator function(little-Endian). */
void* MemoryManager::getList() {
    auto *ptrr = new uint16_t[hole_list.size()];

    for (int i = 0; i < hole_list.size(); i++) {
        //std::cout << hole_list[i] << " ";
        ptrr[i] = hole_list[i];
    }
    //std::cout << std::endl;

    return ptrr;
}

/* Returns a bit-stream of bits representing whether words
 * are used (1) or free (0).The first two bytes are the
 * size of the bitmap(little-Endian). The rest is the
 * bitmap, word-wise. */
void* MemoryManager::getBitmap() {
    // In the words of the wise: "I'll refactor this later"
    uint8_t adjusted_size = (memory_bitmap[1] - 2)/word_size;
    uint8_t *ptr = new uint8_t[adjusted_size + 2];

    for (int i = 0; i < adjusted_size; i++)
        ptr[i] = 0;

    auto temp = memory_bitmap[0];
    ptr[0] = adjusted_size;
    ptr[1] = temp;

    int bitmap_index = 2;
    int byte_number = 2;
    while (byte_number != adjusted_size + 2) {
        for (int i = 0; i < 8; i++) {
            //std::cout << (unsigned) memory_bitmap[bitmap_index];
            ptr[byte_number] |= memory_bitmap[bitmap_index++] << i;
        }
        //std::cout << std::endl;
        ++byte_number;
    }


    return ptr;
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
    return memory_byte_blocks;
}

void MemoryManager::updateHoleList() {
    uint16_t temp_offset = 0, temp_length = 0;
    std::vector<uint16_t> temp_vect;

    for (int i = 2; i < bitmap_size; i++) {
        if (memory_bitmap[i] == 0) {
            temp_offset = (uint16_t )i;
            temp_length = 0;
            while (i < bitmap_size) {
                if (memory_bitmap[i] == 1) {
                    break;
                } else {
                    ++temp_length;
                    ++i;
                }
            }
            temp_vect.push_back(temp_offset - 2);
            temp_vect.push_back(temp_length);
        }
    }

    auto temp_size = static_cast<uint16_t>(temp_vect.size() * 2);
    temp_vect.insert(temp_vect.begin(),temp_size);
    hole_list.swap(temp_vect);
}

void MemoryManager::updateProcList(int start, int len, bool removing) {
    // Check if we are removing a process or adding one to the list
    if (removing) {
        // We are removing a process. Let's check if the process list is empty
        if (proc_list.empty())
            // The list is empty, let's call it a day and return
            return;
        else {
            // The list is not empty, we will now find where this process offset is in the vector
            std::vector<uint16_t >::iterator it;

            // Travers process list to find the potential address we will delete from the process vector
            for (it = proc_list.begin(); it != proc_list.end(); it++) {
                // Check to see if the offset entry exists in the process list
                if (*it == start)
                    // Stop traversing process list if we find it
                    break;
                else
                    it++;
            }

            // Check one more time to see if we didn't find it. If we didn't, we don't do anything
            if (it >= proc_list.end())
                return;

            // If we are here is because we found the entry and we are now going to erase it and the length entry too
            proc_list.erase(it);
            proc_list.erase(it);
        }
    } else { // We are NOT deleting a process! We are adding it!
        proc_list.push_back(static_cast<unsigned short &&>(start));
        proc_list.push_back(static_cast<unsigned short &&>(len));
    }
}

void MemoryManager::updateBitmap(int start, int last, bool allocate) {
    bool temp;
    if (allocate)
        temp = USED;
    else
        temp = FREE;

    for (int i = start; i < last; i++)
        memory_bitmap[i] = static_cast<uint8_t>(temp);
}



/***************** Memory Allocation Algorithms **********************/
/* Returns word offset of hole selected by the best fit
 * memory allocation algorithm, and -1 if there is no fit. */
int bestFit(int size, void *list) {
    int offset = -1, smallest_length = 2000000000;
    std::vector<uint16_t> &_list = *(static_cast<std::vector<uint16_t>*>(list));
    uint16_t current_offset_in_vector = 0;
    uint16_t current_len_in_vector = 0;
    /* Check all holes to find smallest hole we can use to fit the
     * desired block of memory */
    for (int i = 1; i < _list.size(); i += 2) {
        /* Does the block we are trying to allocate fit in the current
         * hole ? */
        current_offset_in_vector = _list[i];
        current_len_in_vector = _list[i + 1];
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
    std::vector<uint16_t> &_list = *(static_cast<std::vector<uint16_t>*>(list));

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