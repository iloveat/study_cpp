#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <assert.h>
#include "src/emApiKernel.h"
#include "model.h"
#include "parser.h"
#include <iostream>
#include <string>
#include <sys/time.h>

using namespace std;


//#define emTTS_HEAP_SIZE	(64<<10)	//LSP堆内存的最低阀值				 (MGC的还会多出20K)
#define emTTS_HEAP_SIZE		(124<<10)	//LSP堆内存的最低阀值，一般要多出20K，(MGC的还会多出20K)
#define emTTS_CACHE_SIZE	(512)
#define emTTS_CACHE_COUNT	(64)
#define emTTS_CACHE_EXT		(4)

// 本次合成的音频文件大小
emInt32	nTotalWaveLen;
// 本次合成的音频文件数据
char *memWavFile = NULL;
// 是否写音频文件的首部，写的话则为WAV格式，否则为PCM格式
bool bIfWriteHead = true;
// 模型文件指针
char *pModelBuffer = NULL;
// 全局停止TTS
emBool global_bStopTTS = emFalse;


// 加载模型参数到内存
void loadModel()
{
    string params = recoverFromString(str_bin);
    int len = params.length();
    pModelBuffer = (char*)malloc(sizeof(char)*(len+1)); //定义数组长度
    memcpy(pModelBuffer, params.c_str(), len);
    pModelBuffer[len] = '\0';
    params.clear();
}

//读取资源回调函数
void emCall CBReadRes(
    emPointer		pParameter,			//[in]用户资源回调指针
    emPointer		pBuffer,			//[out]读取资源存放的buffer
    emResAddress	iPos,				//[in]读取的起始地址
    emResSize		nSize )				//[in]读取的大小
{
    if(pModelBuffer != NULL)
    {
        // 从内存中直接读取模型参数
        memcpy(pBuffer, pModelBuffer+iPos, nSize);
    }
}

//音频输出回调函数
emTTSErrID CBOutputPCM( emPointer pParameter,   //[in] 用户在调用函数emTTS_Create时指定的第4个回调参数
        emUInt16		nCode,                  //[in] 输出音频数据的格式码
        emPByte			pcData,                 //[in] 输出音频数据缓冲区指针
        emSize			nSize )                 //[in] 音频数据长度（以字节为单位）
{
    //printf("CBOutputPCM: %d\n", nSize);
    nTotalWaveLen += nSize;

    if(memWavFile != NULL)
    {
        if(bIfWriteHead)  // WAV
        {
            char *wav_buff = (char*)malloc(sizeof(char)*(44+nTotalWaveLen));
            memcpy(wav_buff, memWavFile, sizeof(char)*(44+nTotalWaveLen-nSize));
            memcpy(wav_buff+sizeof(char)*(44+nTotalWaveLen-nSize), pcData, sizeof(char)*nSize);
            free(memWavFile);
            memWavFile = wav_buff;
        }
        else  // PCM
        {
            char *wav_buff = (char*)malloc(sizeof(char)*(nTotalWaveLen));
            memcpy(wav_buff, memWavFile, sizeof(char)*(nTotalWaveLen-nSize));
            memcpy(wav_buff+sizeof(char)*(nTotalWaveLen-nSize), pcData, sizeof(char)*nSize);
            free(memWavFile);
            memWavFile = wav_buff;
        }
    }
    else
    {
        if(!bIfWriteHead)  // PCM
        {
            memWavFile = (char*)malloc(sizeof(char)*(nSize));
            memcpy(memWavFile, pcData, sizeof(char)*(nSize));
        }
    }

    return emTTS_ERR_OK;
}

//进度回调函数
emTTSErrID CBGetProgress( emPointer pParameter,
                         emUInt32 iProcBegin,
                         emUInt32 nProcLen )
{
    return emTTS_ERR_OK;
}

void emCall	WriteWaveLen(emInt32 nSize)
{
    if(memWavFile != NULL)
    {
        *(int *)(memWavFile+4) = nSize + 32;
        *(int *)(memWavFile+40) = nSize;
    }
}

void WriteWaveHead(int nWaveLength)  //16K采样率
{
    int nHeadSize = 44;
    char *pHead;

    pHead = (char*)malloc(sizeof(char)*nHeadSize);

    strcpy((char*)pHead,"RIFF");
    *(int *)(pHead+4) = nWaveLength*2+32;
    strcpy((char*)(pHead+8),"WAVEfmt ");

    *(int *)(pHead+16) 		= 16;
    *(short *)(pHead+20) 	= 1;					/*WAVE_FORMAT_PCM*/
    *(short *)(pHead+22) 	= 1;					/*single channel wave*/
    *(int *)(pHead+24) 		= 16000;				/*16k sample rate*/
    *(int *)(pHead+28) 		= 16000*2; 				/*average bytes per second*/
    *(short *)(pHead+32) 	= 16/8;
    *(short *)(pHead+34) 	= 16;					/*16 bits per sample*/

    strcpy((char*)(pHead+36),"data");
    *(int *)(pHead+40) 	= nWaveLength*2-44;

    memWavFile = (char*)malloc(sizeof(char)*44);
    memcpy(memWavFile, pHead, sizeof(char)*44);
    free(pHead);

    return;
}

void initTTS(emPByte &mpHeap, emTResPackDesc &mtResPackDesc)
{
    loadModel();
    assert(pModelBuffer != NULL);

    mpHeap = (emPByte)malloc(emTTS_HEAP_SIZE);
    memset(mpHeap, 0, emTTS_HEAP_SIZE);

    //mtResPackDesc.pCBParam = fopen(model_path, "rb");
    //为了初始化成功，必须强制指定一个非空的文件指针
    mtResPackDesc.pCBParam = (FILE*)0x1;
    assert(mtResPackDesc.pCBParam != NULL);
    mtResPackDesc.pfnRead = CBReadRes;
    mtResPackDesc.nSize = 0;
    mtResPackDesc.pCacheBlockIndex = (unsigned char*)malloc((emTTS_CACHE_COUNT+emTTS_CACHE_EXT)*sizeof(char));
    mtResPackDesc.pCacheBuffer = (unsigned char*)malloc((emTTS_CACHE_COUNT+emTTS_CACHE_EXT)*(emTTS_CACHE_SIZE)*sizeof(char));
    mtResPackDesc.nCacheBlockSize = emTTS_CACHE_SIZE;
    mtResPackDesc.nCacheBlockCount = emTTS_CACHE_COUNT;
    mtResPackDesc.nCacheBlockExt = emTTS_CACHE_EXT;
}

inline long long currentTimeInMilliseconds()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

void synthAudio(emPByte &mpHeap, emTResPackDesc &mtResPackDesc, const char *text2synthesize)
{
    nTotalWaveLen = 0;
    if(memWavFile != NULL)
    {
        free(memWavFile);
        memWavFile = NULL;
    }

    // 写WAV文件头
    if(bIfWriteHead)
    {
        WriteWaveHead(0);
    }

    // 创建TTS实例
    emHTTS mhTTS;
    emTTSErrID mReturn = emTTS_Create(&mhTTS, (emPointer)mpHeap, emTTS_HEAP_SIZE, NULL, (emPResPackDesc)&mtResPackDesc, (emSize)1);
    assert(mReturn == emTTS_ERR_OK);

    // 设置音频输出回调
    //mReturn = emTTS_SetParam(mhTTS, emTTS_PARAM_OUTPUT_CALLBACK, (long unsigned int)CBOutputPCM);
    //assert(mReturn == emTTS_ERR_OK);

    //mReturn = emTTS_SetParam(mhTTS, emTTS_PARAM_ROLE, emTTS_USE_ROLE_Virtual_55);
    //assert(mReturn == emTTS_ERR_OK);

    // 设置进度输出回调
    //mReturn = emTTS_SetParam(mhTTS, emTTS_PARAM_PROGRESS_CALLBACK, (long unsigned int)CBGetProgress);
    //assert(mReturn == emTTS_ERR_OK);

    long long t1 = currentTimeInMilliseconds();
    mReturn = emTTS_SynthText(mhTTS, text2synthesize, -1);
    assert(mReturn == emTTS_ERR_OK);
    long long t2 = currentTimeInMilliseconds();
    cout<<"Total emTTS_SynthText time: "<<t2-t1<<endl;

    // 更新WAV文件长度
    if(bIfWriteHead)
    {
        WriteWaveLen(nTotalWaveLen);
    }

    emTTS_Destroy(mhTTS);
}

void safeFree(emPByte &mpHeap, emTResPackDesc &mtResPackDesc)
{
    if(pModelBuffer != NULL)
    {
        free(pModelBuffer);
        pModelBuffer = NULL;
    }

    if(mpHeap != NULL)
    {
        free(mpHeap);
        mpHeap = NULL;
    }

    if(mtResPackDesc.pCacheBuffer != NULL)
    {
        free(mtResPackDesc.pCacheBuffer);
        mtResPackDesc.pCacheBuffer = NULL;
    }

    if(mtResPackDesc.pCacheBlockIndex != NULL)
    {
        free(mtResPackDesc.pCacheBlockIndex);
        mtResPackDesc.pCacheBlockIndex = NULL;
    }

    if(memWavFile != NULL)
    {
        free(memWavFile);
        memWavFile = NULL;
    }
}

void loadModel(const char *filename)
{
    FILE *pFile = fopen(filename, "rb");
    fseek(pFile, 0, SEEK_END); //把指针移动到文件的结尾，获取文件长度
    int len = ftell(pFile); //获取文件长度
    pModelBuffer = (char*)malloc(sizeof(char)*(len+1)); //定义数组长度
    rewind(pFile); //把指针移动到文件开头
    fread(pModelBuffer, 1, len, pFile); //读文件
    pModelBuffer[len] = '\0';
    fclose(pFile);
}

void tts_test(char *text)
{
    emPByte pHeap = NULL;
    emTResPackDesc tResPackDesc;
    char *text2synthesize = (char*)malloc(strlen(text)+1);
    memcpy(text2synthesize, text, strlen(text)+1);

    initTTS(pHeap, tResPackDesc);
    synthAudio(pHeap, tResPackDesc, text2synthesize);
/*
    if(bIfWriteHead)  // WAV
    {
        FILE *bufferedWavFile = fopen("buffered.wav", "wb+");
        fwrite(memWavFile, 1, nTotalWaveLen+44, bufferedWavFile);
        fclose(bufferedWavFile);
    }
    else  // PCM
    {
        FILE *bufferedWavFile = fopen("buffered.pcm", "wb+");
        fwrite(memWavFile, 1, nTotalWaveLen, bufferedWavFile);
        fclose(bufferedWavFile);
    }
*/
    safeFree(pHeap, tResPackDesc);
    free(text2synthesize);
}



