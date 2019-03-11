#include <jni.h>
#include <string>
#include <sstream>
#include <exception>
#include <stdexcept>
#include <iostream>
#include "MemoryManager.h"
# define OUT_PATH "mnt/ubuntu/home/reptilian/logs/"

MemoryManager mm = MemoryManager(2, bestFit);
bool initialized = false;
int used = 0;
int freee = 0;
int space = 0;

// Provided example native call
extern "C"
JNIEXPORT jstring JNICALL
Java_edu_ufl_cise_os_p2_P2Activity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}


extern "C"
JNIEXPORT void JNICALL
Java_edu_ufl_cise_os_p2_P2Activity_initMemoryManager(JNIEnv *env,jobject obj,jint maxAllocationSize){
    // Function should initialize your Memory Manager object with the specified word size
    if (!initialized) {
        mm.initialize((uint32_t)maxAllocationSize);
        space += maxAllocationSize;
        initialized = true;
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_edu_ufl_cise_os_p2_P2Activity_deleteMemoryManager(JNIEnv *env,jobject){
    // Function should release all resources held by the Memory Manager
    if (initialized) {
        mm.shutdown();
        used = 0;
        freee = 0;
        initialized = false;
    }
}

extern "C"
JNIEXPORT jint JNICALL
Java_edu_ufl_cise_os_p2_P2Activity_getFreeSize(JNIEnv *env,jobject){
    // Function returns the word size of the free list
    uint16_t *free_list = (uint16_t*)mm.getList();
    int count = 0;

    if (free_list[0] < 4) {
        count = 0;
        return 0;
    } else {

        for (int i = 2; i <= free_list[0] / 2; i += 2) {
            count += free_list[i];
        }

        freee = count;
        return freee;
    }
}

extern "C"
JNIEXPORT jint JNICALL
Java_edu_ufl_cise_os_p2_P2Activity_getUseSize(JNIEnv *env,jobject){
    used = space - freee;
    return used;
}

extern "C"
JNIEXPORT jint JNICALL
Java_edu_ufl_cise_os_p2_P2Activity_getFragSize(JNIEnv *env,jobject){
    // Function returns the word size of the number of fragments within the Memory Manager
    return space;
}

extern "C"
JNIEXPORT jstring JNICALL
Java_edu_ufl_cise_os_p2_P2Activity_allocateMemory(JNIEnv *env,jobject,jint size){
    // Function allocates memory in the Memory Manager and returns the address of the starting block
    // If none is available, return "RIP"
    void* val = mm.allocate(size);
    std::string addr;
    if (val == nullptr) {
         addr = "RIP";
    } else {
        int offset = (uint8_t*)val - (uint8_t*)mm.getMemoryStart();
        std::stringstream ss;
        ss << val;
        addr = ss.str();
    }
    return env->NewStringUTF(addr.c_str());
}

extern "C"
JNIEXPORT void JNICALL
Java_edu_ufl_cise_os_p2_P2Activity_freeMemory(JNIEnv *env,jobject, jstring addr){
    const char* str_addr = env->GetStringUTFChars(addr, NULL);
    long int free_addr = strtol(str_addr, NULL, 0);
    void* start_addr = (void*) free_addr;
    mm.free(start_addr);
}

extern "C"
JNIEXPORT void JNICALL
Java_edu_ufl_cise_os_p2_P2Activity_shutdown(JNIEnv *env,jobject){
    // Function frees all in use memory from within the Memory Manager
    if (initialized) {
        mm.shutdown();
        initialized = false;
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_edu_ufl_cise_os_p2_P2Activity_setAlgorithm(JNIEnv *env,jobject, jint alg){
    // Functions changes the internal allocation algorithm used within your Memory Manager
    // 1 denotes Best Fit, 2 denotes Worst fit
    if (alg == 1)
        mm.setAllocator(bestFit);
    else if (alg == 2)
        mm.setAllocator(worstFit);
}

extern "C"
JNIEXPORT void JNICALL
Java_edu_ufl_cise_os_p2_P2Activity_writeLogs(JNIEnv *env,jobject){
    // TODO:
    // Use your POSIX calls to write logs to file at OUT_PATH that represent the holes in memory
}




