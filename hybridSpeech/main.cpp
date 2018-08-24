#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "src/emApiKernel.h"
#include "interface.h"
#include "src/jnirelated.h"


void jniTest()
{
    JavaVMInitArgs vm_args;
    vm_args.version = JNI_VERSION_1_2;
    vm_args.nOptions = 0;
    vm_args.ignoreUnrecognized = 1;

    JavaVM *jvm = NULL;
    JNIEnv *env = NULL;

    // Construct a VM
    JNI_CreateJavaVM(&jvm, (void **)&env, &vm_args);

    // Construct a String
    jstring jstr = env->NewStringUTF("Hello World");

    // First get the class that contains the method you need to call
    jclass clazz = env->FindClass("java/lang/String");

    // Get the method that you want to call
    jmethodID to_lower = env->GetMethodID(clazz, "toLowerCase", "()Ljava/lang/String;");
    // Call the method on the object
    jobject result = env->CallObjectMethod(jstr, to_lower);

    // Get a C-style string
    const char* str = env->GetStringUTFChars((jstring)result, NULL);
    printf("%s\n", str);

    // Clean up
    env->ReleaseStringUTFChars(jstr, str);

    // Shutdown the VM
    jvm->DestroyJavaVM();
}


int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        printf("usage ./ttsdemo [text2synthesize]\n");
        return -1;
    }

//    if(initJavaVM() < 0)
//    {
//        printf("initial JavaVM failed\n");
//        return -1;
//    }

    tts_test(argv[1]);

//    freeJavaVM();

    return 0;
}











