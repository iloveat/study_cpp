# distutils: language = c++
# distutils: sources = world_stream.cpp

# Cython interface file for wrapping the object

from libcpp.vector cimport vector

# c++ interface to cython
cdef extern from "world_stream.h":# namespace "yuhao":
	cdef cppclass StreamVocoder:
		StreamVocoder(vector[double], int, int, int, int, int, int, double, int)
		vector[short] get_pcm_stream()

cdef class ExVocoderStream:
	cdef StreamVocoder *thisptr
	def __cinit__(self, params, mgc_dim, bap_dim, num_frame, sample_rate, fft_size, buffer_size, mgc_alpha, mgc2sp_omp_threads):
		self.thisptr = new StreamVocoder(params, mgc_dim, bap_dim, num_frame, sample_rate, fft_size, buffer_size, mgc_alpha, mgc2sp_omp_threads)
	def __dealloc__(self):
		del self.thisptr
	def get_pcm_stream(self):
		return self.thisptr.get_pcm_stream()
	
