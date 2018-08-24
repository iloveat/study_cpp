#ifndef INTERFACE
#define INTERFACE

#include <iostream>

using namespace std;


extern "C"
{
	int add(int a, int b);

    void synthesizeFromParamsFile(
            const int &offline,        //offline:1, online:0
            const int &num_spectrum,   //dimension of mgc, offline:19, online:59
            const int &num_skip,       //skipped points
            const double &world_alpha, //parameter alpha of world
            const char *pinput,        //(input) parameter file name
            const char *poutput);      //(output) pcm file name
}

#endif // INTERFACE

