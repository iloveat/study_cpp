# -*- coding: utf-8 -*- 
import ctypes

library_handler = ctypes.CDLL('libgenLab.so')
generate_lab = library_handler.gen_lab
generate_lab.argtypes = [ctypes.c_char_p]
generate_lab.restype = ctypes.c_char_p

output = generate_lab('邹.城'.decode('utf-8').encode('gbk'))
print output
