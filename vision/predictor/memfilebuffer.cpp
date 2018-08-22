/*!
 * \file memfilebuffer.cpp
 * \brief read file into memory buffer
 * \author zoucheng
 * \date 2017.05.12
 */

#include "memfilebuffer.h"
#include <iostream>
#include <string>
#include <fstream>
#include <string.h>
#include <assert.h>

using namespace std;


MemFileBuffer::MemFileBuffer(const std::string &file_path)
{
    std::ifstream inf(file_path.c_str(), std::ios::in | std::ios::binary);
    if(!inf)
    {
        std::cerr<<"Can't open the file. Please check " <<file_path<< ". \n";
        assert(false);
    }

    inf.seekg(0, std::ios::end);
    mLength = inf.tellg();
    inf.seekg(0, std::ios::beg);

    mBuffer = new char[mLength*sizeof(char)];
    inf.read(mBuffer, mLength);
    inf.close();
}

MemFileBuffer::MemFileBuffer(const char *str_buff, const int &length)
{
    mLength = length;
    mBuffer = new char[mLength*sizeof(char)];
    memcpy(mBuffer, str_buff, mLength*sizeof(char));
}

MemFileBuffer::~MemFileBuffer()
{
    if(mBuffer != NULL)
    {
        delete []mBuffer;
        mBuffer = NULL;
    }
    mLength = 0;
}

char* MemFileBuffer::getBuffer()
{
    return mBuffer;
}

int MemFileBuffer::getLength()
{
    return mLength;
}







