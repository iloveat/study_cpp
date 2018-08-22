/*!
 * \file memfilebuffer.h
 * \brief read file into memory buffer
 * \author zoucheng
 * \date 2017.05.12
 */

#ifndef MEMFILEBUFFER_H
#define MEMFILEBUFFER_H

#include <iostream>
#include <string>

using namespace std;


class MemFileBuffer
{
private:
    char *mBuffer;
    int mLength;

public:
    MemFileBuffer(const std::string &file_path);

    MemFileBuffer(const char *str_buff, const int &length);

    ~MemFileBuffer();

    char* getBuffer();

    int getLength();
};

#endif // MEMFILEBUFFER_H
