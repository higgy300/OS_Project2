#include <jni.h>
#include <string>
#include <sstream>
#include <exception>
#include <stdexcept>
#include <iostream>
#include "MemoryManager.h"
# define OUT_PATH "mnt/ubuntu/home/reptilian/logs/"


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
Java_edu_ufl_cise_os_p2_P2Activity_initMemoryManager(JNIEnv *env,jobject,jint maxAllocationSize){
    // TODO:
    // Function should initialize your Memory Manager object with the specified word size;
    MemoryManager mm = MemoryManager(maxAllocationSize, nullptr_t);



}

extern "C"
JNIEXPORT void JNICALL
Java_edu_ufl_cise_os_p2_P2Activity_deleteMemoryManager(JNIEnv *env,jobject){
    // TODO:
    // Function should release all resources held by the Memory Manager
}

extern "C"
JNIEXPORT jint JNICALL
Java_edu_ufl_cise_os_p2_P2Activity_getFreeSize(JNIEnv *env,jobject){
    // TODO:
    // Function returns the word size of the free list
    return 0;
}

extern "C"
JNIEXPORT jint JNICALL
Java_edu_ufl_cise_os_p2_P2Activity_getUseSize(JNIEnv *env,jobject){
    // TODO:
    // Function returns the word size of the in use list
    return 0;
}

extern "C"
JNIEXPORT jint JNICALL
Java_edu_ufl_cise_os_p2_P2Activity_getFragSize(JNIEnv *env,jobject){
    // TODO:
    // Function returns the word size of the number of fragments within the Memory Manager
    return 0;
}

extern "C"
JNIEXPORT jstring JNICALL
Java_edu_ufl_cise_os_p2_P2Activity_allocateMemory(JNIEnv *env,jobject,jint size){
    // TODO:
    // Function allocates memory in the Memory Manager and returns the address of the starting block
    // If none is available, return "RIP"
    std::string addr = "RIP";
    return env->NewStringUTF(addr.c_str());
}

extern "C"
JNIEXPORT void JNICALL
Java_edu_ufl_cise_os_p2_P2Activity_freeMemory(JNIEnv *env,jobject, jstring addr){
    // TODO:
    // Function frees block at specified address within the Memory Manager
}

extern "C"
JNIEXPORT void JNICALL
Java_edu_ufl_cise_os_p2_P2Activity_shutdown(JNIEnv *env,jobject){
    // TODO:
    // Function frees all in use memory from within the Memory Manager
}

extern "C"
JNIEXPORT void JNICALL
Java_edu_ufl_cise_os_p2_P2Activity_setAlgorithm(JNIEnv *env,jobject, jint alg){
    // TODO:
    // Functions changes the internal allocation algorithm used within your Memory Manager
    // 1 denotes Best Fit, 2 denotes Worst fit
}

extern "C"
JNIEXPORT void JNICALL
Java_edu_ufl_cise_os_p2_P2Activity_writeLogs(JNIEnv *env,jobject){
    // TODO:
    // Use your POSIX calls to write logs to file at OUT_PATH that represent the holes in memory
}




