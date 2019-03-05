#include "MemoryManager.h"
#include <string>
#include <cmath>
#include <array>
#include <sstream>
#include <fstream>
#include <vector>
#include <iostream>

// Prototypes
void testFree();
void testBestFit();
void testP2Document();
void printBits(std::stringstream& ss, uint8_t number, uint8_t bitsInNumber);
void testGetBitmap(MemoryManager& memoryManager, uint16_t correctBitmapLength, std::string correctBitmap);
void testGetList(MemoryManager& memoryManager, uint16_t correctListLength, std::vector<uint16_t> correctList);
void testGetWordSize(MemoryManager& memoryManager, size_t correctWordSize);
void testGetMemoryLimit(MemoryManager& memoryManager, size_t correctMemoryLimit);
void testDumpMemoryMap(MemoryManager& memoryManager, std::string correctFileContents);



// Program entrypoint

int main() 
{
    //testFree();
    testBestFit();
    //testP2Document();
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
    std::cout << testArray2 << std::endl;
    memoryManager.free(testArray2);
    std::cout << testArray2 << std::endl;

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
void testBestFit()
{
    size_t numberOfWords = 96;
    MemoryManager memoryManager(8, worstFit);
    memoryManager.initialize(numberOfWords);
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

    std::string correctBitmap =
    "00001111\n11111110\n00000011\n00000000\n00000000\n00000000\n00000000\n00000000\n00000000\n00000000\n00000000\n00000000\n";
    uint16_t correctBitmapLength = 12;


    std::vector<uint16_t> correctList = {4,5,18,78};
    uint16_t correctListLength = correctList.size() * 2;


    size_t correctWordSize = 8;


    size_t correctMemoryLimit = 8 * numberOfWords;


    std::string correctFileContents = "[4, 5] - [18, 78]";


    testGetBitmap(memoryManager, correctBitmapLength, correctBitmap);  
    testGetList(memoryManager, correctListLength, correctList);
    testGetWordSize(memoryManager, correctWordSize);
    testGetMemoryLimit(memoryManager, correctMemoryLimit);
    //testDumpMemoryMap(memoryManager, correctFileContents);

    memoryManager.shutdown();
}
// Performs the example given in the P2 documents
void testP2Document()
{
    std::cout << "\n\nTest Case: p2DocumentTest" << std::endl;

    size_t numberOfWords = 26;
    MemoryManager memoryManager(8, bestFit);
    memoryManager.initialize(numberOfWords);

    uint64_t* testArray1 = static_cast<uint64_t*>(memoryManager.allocate(sizeof(uint64_t) * 10));
    uint64_t* testArray2 = static_cast<uint64_t*>(memoryManager.allocate(sizeof(uint64_t) * 2));
    uint64_t* testArray3 = static_cast<uint64_t*>(memoryManager.allocate(sizeof(uint64_t) * 2));
    uint64_t* testArray4 = static_cast<uint64_t*>(memoryManager.allocate(sizeof(uint64_t) * 6));

    memoryManager.free(testArray1);
    memoryManager.free(testArray3);

    std::string correctBitmap =
    "00000000\n11001100\n00001111\n00000000";
    uint16_t correctBitmapLength = 4;

    std::vector<uint16_t> correctList = {0, 10, 12, 2, 20, 6};
    uint16_t correctListLength = correctList.size() * 2;

    size_t correctWordSize = 8;

    size_t correctMemoryLimit = 8 * numberOfWords;

    std::string correctFileContents = "[0, 10] - [12, 2] - [20, 6]";

    testGetBitmap(memoryManager, correctBitmapLength, correctBitmap);
    testGetList(memoryManager, correctListLength, correctList);
    testGetWordSize(memoryManager, correctWordSize);
    testGetMemoryLimit(memoryManager, correctMemoryLimit);
    //testDumpMemoryMap(memoryManager, correctFileContents);
}

void printBits(std::stringstream& ss, uint8_t number, uint8_t bitsInNumber) 
{ 
    for (uint16_t i = 1 << bitsInNumber - 1; i > 0; i = i / 2) {
        (number & i)? ss << "1": ss << "0"; 
    }
    ss << std::endl;
}

void testGetBitmap(MemoryManager& memoryManager, uint16_t correctBitmapLength, std::string correctBitmap)
{
    std::cout << "\nTesting getBitmap()\n\n" << std::endl;
    uint8_t* bitmap = static_cast<uint8_t*>(memoryManager.getBitmap());
    uint8_t* bitmapEntryPoint = bitmap;

    uint8_t lowerByte = *bitmap++;
    uint8_t higherByte = *bitmap++;
    uint16_t byteStreamLength = (higherByte << 8) | lowerByte;
    std::cout << "Your result is:" << std::endl;
    std::cout << "Bitmap length: " <<  byteStreamLength << std::endl;
    
    std::stringstream ss;

    for(uint16_t i = 0; i < byteStreamLength; ++i) {
        printBits(ss, bitmap[i], 8);  
    }

    std::cout << ss.str();
    std::cout << "\nThe correct result is: " << std::endl;
    std::cout << "Bitmap length: " << correctBitmapLength << std::endl;
    std::cout << correctBitmap << std::endl;

    delete [] bitmapEntryPoint;
}


void testGetList(MemoryManager& memoryManager, uint16_t correctListLength, std::vector<uint16_t> correctList)
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


    std::cout << "The correct result is : " << std::endl;
    std::cout << "List length: "  << correctListLength << std::endl;

    for(auto element: correctList) {
        std::cout << element << " ";
    }
    std::cout << std::endl;

    delete [] listEntryPoint;
}

void testGetWordSize(MemoryManager& memoryManager, size_t correctWordSize)
{
    std::cout << "\n\nTesting getWordSize()\n\n" << std::endl;
    std::cout << "Your result is: " << memoryManager.getWordSize() << "\n" << std::endl;
    std::cout << "The correct result is: " << correctWordSize << std::endl;
}

void testGetMemoryLimit(MemoryManager& memoryManager, size_t correctMemoryLimit)
{
    std::cout << "\n\nTesting getMemoryLimit()\n\n" << std::endl;
    std::cout << "Your result is: " << memoryManager.getMemoryLimit() << "\n" <<std::endl;
    std::cout << "The correct result is: " << correctMemoryLimit << std::endl;
}

void testDumpMemoryMap(MemoryManager& memoryManager, std::string correctFileContents)
{
    std::cout << "\n\nTesting dumpMemoryMap()\n\n" << std::endl;
    memoryManager.dumpMemoryMap((char*)"test.txt");
    std::ifstream testFile("test.txt");
    if (testFile.is_open()) {
        std::string line;
        std::getline(testFile, line);
        testFile.close();

        std::cout << "Your result is: \n" << line << "\n" << std::endl;

        std::cout << "The correct result is: \n" << correctFileContents << std::endl;
    }
}

