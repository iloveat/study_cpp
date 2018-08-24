SOURCES_C += ../src/emADPCM.c
SOURCES_C += ../src/emCommon.c
SOURCES_C += ../src/emDebug.c
SOURCES_C += ../src/emDecode.c
SOURCES_C += ../src/emG7231.c
SOURCES_C += ../src/emG7231_COD_CNG.c
SOURCES_C += ../src/emG7231_CODER.c
SOURCES_C += ../src/emG7231_DEC_CNG.c
SOURCES_C += ../src/emG7231_DECOD.c
SOURCES_C += ../src/emG7231_EXC_LBC.c
SOURCES_C += ../src/emG7231_EXC_LBC_Cache.c
SOURCES_C += ../src/emG7231_LPC.c
SOURCES_C += ../src/emG7231_LPC_Cache.c
SOURCES_C += ../src/emG7231_LSP.c
SOURCES_C += ../src/emG7231_LSP_Cache.c
SOURCES_C += ../src/emG7231_TAB_LBC.c
SOURCES_C += ../src/emG7231_TAME.c
SOURCES_C += ../src/emG7231_TYPEDEF.c
SOURCES_C += ../src/emG7231_UTIL_CNG.c
SOURCES_C += ../src/emG7231_UTIL_LBC.c
SOURCES_C += ../src/emG7231_VAD.c
SOURCES_C += ../src/emHeap.c
SOURCES_C += ../src/emMath.c
SOURCES_C += ../src/emResample.c
SOURCES_C += ../src/emRes.c
SOURCES_C += ../src/emResPack.c
SOURCES_C += ../src/emString.c
SOURCES_C += ../src/emTTS.c
SOURCES_C += ../src/emTTS_Common.c
SOURCES_C += ../src/Front_CodeConvert.c
SOURCES_C += ../src/Front_ProcFuHao.c
SOURCES_C += ../src/Front_ProcHZRhythm_New.c
SOURCES_C += ../src/Front_ProcHZRhythm_New_PPH.c
SOURCES_C += ../src/Front_ProcHZRhythm_New_PW.c
SOURCES_C += ../src/Front_ProcHZRhythm_Old.c
SOURCES_C += ../src/Front_ProcHZToPinYin.c
SOURCES_C += ../src/Front_ProcHZWordSeg.c
SOURCES_C += ../src/Front_ProcShuZi.c
SOURCES_C += ../src/Front_ProcYingWen.c
SOURCES_C += ../src/Front_ToLab.c
SOURCES_C += ../src/Rear_Common.c
SOURCES_C += ../src/Rear_Pre.c
SOURCES_C += ../src/Rear_Synth.c
SOURCES_C += ../src/vocoder/common.c
SOURCES_C += ../src/vocoder/fft.c
SOURCES_C += ../src/vocoder/matlabfuns.c
SOURCES_C += ../src/vocoder/sptk.c
SOURCES_C += ../src/vocoder/synthesis.c
SOURCES_C += ../src/vocoder/world.c

SOURCES_CPP += ../main.cpp


TARGET_NAME = txt2lab 


XXFLAGS += -Wall -W -fPIC


OBJS_C = $(SOURCES_C:.c=.o)
OBJS_CPP = $(SOURCES_CPP:.cpp=.o)


$(TARGET_NAME) : $(OBJS_C) $(OBJS_CPP)
	g++ $(XXFLAGS) -o $@ $^


%.o : %.c
	gcc $(XXFLAGS) -o $@ -c $<


%.o : %.cpp
	g++ $(XXFLAGS) -o $@ -c $<



clean :
	rm -f $(OBJS_C) $(OBJS_CPP) $(TARGET_NAME)


.PHONY : clean


























