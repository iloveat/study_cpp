#ifndef INTERFACE
#define INTERFACE

#include <iostream>
#include "Python.h"

using namespace std;


extern "C"
{
	void initExVocoder16K();
	static PyObject* ExVocoder16K_add(PyObject *self, PyObject *args);
	static PyObject* ExVocoder16K_array(PyObject *self, PyObject *args);
	static PyObject* ExVocoder16K_synthesizeFileFileWAV(PyObject *self, PyObject *args);
	static PyObject* ExVocoder16K_synthesizeMemoFileWAV(PyObject *self, PyObject *args);
	static PyObject* ExVocoder16K_synthesizeMemoMemoPCM(PyObject *self, PyObject *args);
}

#endif // INTERFACE

