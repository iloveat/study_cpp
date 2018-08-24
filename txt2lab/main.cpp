#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "interface.h"
extern char strLab[20000];


char* obtain_lab(char* text)
{
    if(text == NULL)
        return "";

    tts_test(text);

    return strLab;
}


int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        return -1;
    }

    printf(obtain_lab(argv[1]));

    return 0;
}




























