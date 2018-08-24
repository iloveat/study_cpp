from distutils.core import setup, Extension
from Cython.Distutils import build_ext


module_name = 'ExVocoder_stream_cython'

source_file = [
	'../../world-master/src/common.cpp', 
	'../../world-master/src/fft.cpp', 
	'../../world-master/src/matlabfunctions.cpp', 
	'../../world-master/src/synthesisrealtime.cpp',
	'../../world-master/src/sptk.cpp', 
	'world_stream.cpp', 
	'ExVocoder_stream_cython.pyx'
]

modules = [Extension(module_name, source_file, extra_compile_args=['-std=c++11','-O2','-fopenmp','-lpthread'], language='c++', extra_link_args=['-fopenmp'])]

setup(name=module_name, ext_modules=modules, cmdclass = {'build_ext':build_ext})




