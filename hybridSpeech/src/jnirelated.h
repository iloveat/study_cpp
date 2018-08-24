#ifndef JNIRELATED
#define JNIRELATED

#include <jni.h>

JavaVM *global_jvm;
JNIEnv *global_env;


char *jniWordSegment(const char *strGbk)
{
    if(global_env == NULL || strGbk == NULL)
    {
        return NULL;
    }
#ifdef __cplusplus
    JNIEnv *env = global_env;
    jstring jstr = env->NewStringUTF(strGbk);
    jclass clazz = env->FindClass("WordSegment");
    jmethodID splitMID = env->GetStaticMethodID(clazz, "splitGbkSentence", "(Ljava/lang/String;)Ljava/lang/String;");
    jobject result = env->CallStaticObjectMethod(clazz, splitMID, jstr);
    const char* str = env->GetStringUTFChars((jstring)result, NULL);
    env->ReleaseStringUTFChars(jstr, str);
    return (char*)str;
#else
    JNIEnv *env = global_env;
    jstring jstr = (*env)->NewStringUTF(env, strGbk);
    jclass clazz = (*env)->FindClass(env, "WordSegment");
    jmethodID splitMID = (*env)->GetStaticMethodID(env, clazz, "splitGbkSentence", "(Ljava/lang/String;)Ljava/lang/String;");
    jobject result = (*env)->CallStaticObjectMethod(env, clazz, splitMID, jstr);
    char* str = (*env)->GetStringUTFChars(env, (jstring)result, NULL);
    //notice: c version code cannot release the string
    //(*env)->ReleaseStringUTFChars(env, jstr, str);
    return (char*)str;
#endif
}


char *jniSentence2Pinyin(const char *strGbk)
{
    if(global_env == NULL || strGbk == NULL)
    {
        return NULL;
    }
#ifdef __cplusplus
    JNIEnv *env = global_env;
    jstring jstr = env->NewStringUTF(strGbk);
    jclass clazz = env->FindClass("Sentence2Pinyin");
    jmethodID splitMID = env->GetStaticMethodID(clazz, "getPinyin", "(Ljava/lang/String;)Ljava/lang/String;");
    jobject result = env->CallStaticObjectMethod(clazz, splitMID, jstr);
    const char* str = env->GetStringUTFChars((jstring)result, NULL);
    env->ReleaseStringUTFChars(jstr, str);
    return (char*)str;
#else
    JNIEnv *env = global_env;
    jstring jstr = (*env)->NewStringUTF(env, strGbk);
    jclass clazz = (*env)->FindClass(env, "Sentence2Pinyin");
    jmethodID splitMID = (*env)->GetStaticMethodID(env, clazz, "getPinyin", "(Ljava/lang/String;)Ljava/lang/String;");
    jobject result = (*env)->CallStaticObjectMethod(env, clazz, splitMID, jstr);
    char* str = (*env)->GetStringUTFChars(env, (jstring)result, NULL);
    //notice: c version code cannot release the string
    //(*env)->ReleaseStringUTFChars(env, jstr, str);
    return (char*)str;
#endif
}


int initJavaVM()
{
    global_jvm = NULL;
    global_env = NULL;
    char *jarbase = "/home/brycezou/works/java_work/tts_front";
    char classpath[300];
    sprintf(classpath, "-Djava.class.path=%s/%s:%s/%s\0",
            jarbase, "libs/nlp-hanlp-1.2.12-20170720.083037-30.jar",
            jarbase, "out/production/tts_front");

    JavaVMOption options[1] = {
        {classpath, NULL}
    };

    JavaVMInitArgs vm_args;
    vm_args.version = JNI_VERSION_1_2;
    vm_args.nOptions = 1;
    vm_args.options = options;
    vm_args.ignoreUnrecognized = 1;

    int ret = JNI_CreateJavaVM(&global_jvm, (void **)&global_env, &vm_args);
    if(ret < 0)
    {
        return -1;
    }

/*
    // load the class implicitly to save time
    char *str = jniWordSegment("%C4%E3%BA%C3");
    if(str == NULL)
    {
        return -2;
    }
    str = jniSentence2Pinyin("%C4%E3%BA%C3");
    if(str == NULL)
    {
        return -3;
    }
*/

    return 1;
}

void freeJavaVM()
{
    if(global_jvm != NULL)
    {
#ifdef __cplusplus
        global_jvm->DestroyJavaVM();
#else
        (*global_jvm)->DestroyJavaVM(global_jvm);
#endif
        global_jvm = NULL;
        global_env = NULL;
    }
}

#endif // JNIRELATED
