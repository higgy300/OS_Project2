#include "MemoryManager.h"
#include <string>
#include <sstream>
#include <cmath>
#include <array>
#include <fstream>
#include <iostream>

// Prototypes
void testFree();
void simpleMemoryManagerTest();
void printBits(std::stringstream& ss, uint16_t number);
void testGetBitmap(MemoryManager& memoryManager);
void testGetList(MemoryManager& memoryManager);
void testGetWordSize(MemoryManager& memoryManager);
void testGetMemoryLimit(MemoryManager& memoryManager);
void testDumpMemoryMap(MemoryManager& memoryManager);

// Program entrypoint
int main() 
{
    testFree();
    simpleMemoryManagerTest();
}

// This function allocates 200 bytes of memory with block sizes of 40 bytes.
// It then performs deallocations in the middle, then the first and 
// last allocated blocks, and then finally removes the final allocated block in the middle.
void testFree()
{
    MemoryManager memoryManager(8, bestFit);
    memoryManager.initialize(100);
    uint32_t* testArray1 = static_cast<uint32_t*>(memoryManager.allocate(sizeof(uint32_t) * 10));
    uint32_t* testArray2 = static_cast<uint32_t*>(memoryManager.allocate(sizeof(uint32_t) * 10));
    uint32_t* testArray3 = static_cast<uint32_t*>(memoryManager.allocate(sizeof(uint32_t) * 10));
    uint32_t* testArray4 = static_cast<uint32_t*>(memoryManager.allocate(sizeof(uint32_t) * 10));
    uint32_t* testArray5 = static_cast<uint32_t*>(memoryManager.allocate(sizeof(uint32_t) * 10));

    memoryManager.free(testArray2);
    memoryManager.free(testArray4);
    memoryManager.free(testArray5);
    memoryManager.free(testArray1);
    memoryManager.free(testArray3);
	// Tests deconstructor ;)
}

// This function allocates 144 bytes of memory with different size blocks(block sizes are 16 bytes 
// and 40 bytes). It then performs specific deallocations to create holes within the internal block 
// held by memoryManager. Once the holes are created, the allocation algorithm is changed to besFit.
// The appropriate size hole should then be picked once an allocation is made.
// Tests are then run on the state of the memoryManager.
void simpleMemoryManagerTest()
{
    MemoryManager memoryManager(8, worstFit);
    memoryManager.initialize(96);
    // allocate
    uint32_t* testArray1 = static_cast<uint32_t*>(memoryManager.allocate(sizeof(uint32_t) * 4));
    uint32_t* testArray2 = static_cast<uint32_t*>(memoryManager.allocate(sizeof(uint32_t) * 4));
    uint32_t* testArray3 = static_cast<uint32_t*>(memoryManager.allocate(sizeof(uint32_t) * 10));
    uint32_t* testArray4 = static_cast<uint32_t*>(memoryManager.allocate(sizeof(uint32_t) * 4));
    uint32_t* testArray5 = static_cast<uint32_t*>(memoryManager.allocate(sizeof(uint32_t) * 4));
    uint32_t* testArray6 = static_cast<uint32_t*>(memoryManager.allocate(sizeof(uint32_t) * 10));

    // free specific allocations to create holes
    memoryManager.free(testArray3);
    memoryManager.free(testArray5);
    // change allocator
    memoryManager.setAllocator(bestFit);

    uint32_t* testArray7 = static_cast<uint32_t*>(memoryManager.allocate(sizeof(uint32_t) * 4));

    testGetBitmap(memoryManager);
    testGetList(memoryManager);
    testGetWordSize(memoryManager);
    testGetMemoryLimit(memoryManager);
    testDumpMemoryMap(memoryManager);
    memoryManager.shutdown();
}

// 
void printBits(std::stringstream& ss, uint16_t number) 
{ 
    for (uint16_t i = 1 << 15; i > 0; i = i / 2) {
        (number & i)? ss << "1": ss << "0"; 
    }
    ss << std::endl;
}

void testGetBitmap(MemoryManager& memoryManager)
{
    std::cout << "\nTesting getBitmap()\n\n" << std::endl;
    uint16_t* bitmap = static_cast<uint16_t*>(memoryManager.getBitmap());
    uint16_t* bitmapEntryPoint = bitmap;

    uint16_t bitmapLength = *bitmap++;
    std::cout << "Your result is:" << std::endl;
    
    std::cout << "Bitmap length: " <<  bitmapLength << std::endl;
    
    std::stringstream ss;
    uint16_t numberOfBitsInBitmap = bitmapLength * 8;
    for(uint16_t i = 0; i < std::ceil(static_cast<double>(numberOfBitsInBitmap) / 16); ++i) {
        printBits(ss, bitmap[i]);  
    }

    std::cout << ss.str();
    std::stringstream correctBitmap;
    correctBitmap << "1111000001111111\n"; 
    correctBitmap << "1100000000000000\n"; 
    correctBitmap << "0000000000000000\n"; 
    correctBitmap << "0000000000000000\n";
    correctBitmap << "0000000000000000\n";
    correctBitmap << "0000000000000000\n";
    std::cout << "\nThe correct result is: " << std::endl;
    std::cout << "Bitmap length: " << 12 << std::endl;
    std::cout << correctBitmap.str() << std::endl;

    delete [] bitmapEntryPoint;
}

void testGetList(MemoryManager& memoryManager)
{
    std::cout << "\nTesting getList()\n\n" << std::endl;
    uint16_t* list = static_cast<uint16_t*>(memoryManager.getList());
    uint16_t* listEntryPoint = list;

    uint16_t listLength = *list++;

    std::cout << "Your result is:" << std::endl;
    std::cout << "List length: " << listLength << std::endl;

    uint16_t bytesPerEntry = 2;
    uint16_t entriesInList = listLength / bytesPerEntry;
    // print all 2 byte entries in list
    for(uint16_t i = 0; i < entriesInList; ++i) {
        std::cout << list[i] << " ";
    }
    std::cout << "\n" << std::endl;

    std::array<uint16_t, 4> correctListAsArray = {4,5,18,78};

    std::cout << "The correct result is : " << std::endl;
    std::cout << "List length: "  << correctListAsArray.size() * bytesPerEntry << std::endl;

    for(auto element: correctListAsArray) {
        std::cout << element << " ";
    }
    std::cout << std::endl;

    delete [] listEntryPoint;
}

void testGetWordSize(MemoryManager& memoryManager)
{
    std::cout << "\n\nTesting getWordSize()\n\n" << std::endl;
    std::cout << "Your result is: " << memoryManager.getWordSize() << "\n" << std::endl;
    std::cout << "The correct result is: " << 8 << std::endl;
}

void testGetMemoryLimit(MemoryManager& memoryManager)
{
    std::cout << "\n\nTesting getMemoryLimit()\n\n" << std::endl;
    std::cout << "Your result is: " << memoryManager.getMemoryLimit() << "\n" <<std::endl;
    std::cout << "The correct result is: " << 8 * 96 << std::endl;
}

void testDumpMemoryMap(MemoryManager& memoryManager)
{
    std::cout << "\n\nTesting dumpMemoryMap()\n\n" << std::endl;
    memoryManager.dumpMemoryMap((char*)"test.txt");
    std::ifstream testFile("test.txt");
    if (testFile.is_open()) {
        std::string line;
        std::getline(testFile, line);
        testFile.close();

        std::cout << "Your result is: \n" << line << "\n" << std::endl;

        std::string correctOutput = "[4, 5] - [18, 78]";
        std::cout << "The correct result is: \n" << correctOutput << std::endl;
    }
}

