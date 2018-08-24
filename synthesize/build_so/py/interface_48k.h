#ifndef INTERFACE
#define INTERFACE

#include <iostream>
#include "Python.h"

using namespace std;


extern "C"
{
	void initExVocoder48K();
	static PyObject* ExVocoder48K_synthesizeMemoMemoPCM(PyObject *self, PyObject *args);
}

#endif // INTERFACE

