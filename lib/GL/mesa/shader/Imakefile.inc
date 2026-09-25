XCOMM $XFree86:$

#ifndef MesaShaderBuildDir
#define MesaShaderBuildDir $(GLXLIBSRC)/mesa/shader/
#endif

MESASHADERBUILDDIR = MesaShaderBuildDir

  MESA_SHADER_SRCS =	$(MESASHADERBUILDDIR)arbprogparse.c \
			$(MESASHADERBUILDDIR)arbprogram.c \
			$(MESASHADERBUILDDIR)arbfragparse.c \
			$(MESASHADERBUILDDIR)arbvertparse.c \
			$(MESASHADERBUILDDIR)grammar_mesa.c \
			$(MESASHADERBUILDDIR)nvprogram.c \
			$(MESASHADERBUILDDIR)nvfragparse.c \
			$(MESASHADERBUILDDIR)nvvertexec.c \
			$(MESASHADERBUILDDIR)nvvertparse.c \
			$(MESASHADERBUILDDIR)program.c

#ifdef NeedToLinkMesaSrc
LinkSourceFile(arbprogparse.c, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(arbprogram.c, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(arbfragparse.c, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(arbvertparse.c, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(arbfragparse.h, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(arbprogparse.h, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(arbprogram.h, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(arbprogram_syn.h, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(arbvertparse.h, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(nvfragparse.h, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(nvfragprog.h, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(nvprogram.c, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(nvprogram.h, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(nvfragparse.c, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(nvvertexec.c, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(nvvertexec.h, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(nvvertparse.c, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(nvvertparse.h, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(nvvertprog.h, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(grammar.c, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(grammar_mesa.c, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(grammar.h, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(grammar_mesa.h, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(grammar_syn.h, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(program.c, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(program.h, $(MESASRCDIR)/src/mesa/shader)
#endif

  MESA_SHADER_OBJS =	$(MESASHADERBUILDDIR)arbprogparse.o \
			$(MESASHADERBUILDDIR)arbprogram.o \
			$(MESASHADERBUILDDIR)arbfragparse.o \
			$(MESASHADERBUILDDIR)arbvertparse.o \
			$(MESASHADERBUILDDIR)grammar_mesa.o \
			$(MESASHADERBUILDDIR)nvprogram.o \
			$(MESASHADERBUILDDIR)nvfragparse.o \
			$(MESASHADERBUILDDIR)nvvertexec.o \
			$(MESASHADERBUILDDIR)nvvertparse.o \
			$(MESASHADERBUILDDIR)program.o

#if defined(DoSharedLib) && DoSharedLib
  MESA_SHADER_UOBJS =	$(MESASHADERBUILDDIR)unshared/arbprogparse.o \
			$(MESASHADERBUILDDIR)unshared/arbprogram.o \
			$(MESASHADERBUILDDIR)unshared/arbfragparse.o \
			$(MESASHADERBUILDDIR)unshared/arbvertparse.o \
			$(MESASHADERBUILDDIR)unshared/grammar_mesa.o \
			$(MESASHADERBUILDDIR)unshared/nvprogram.o \
			$(MESASHADERBUILDDIR)unshared/nvfragparse.o \
			$(MESASHADERBUILDDIR)unshared/nvvertexec.o \
			$(MESASHADERBUILDDIR)unshared/nvvertparse.o \
			$(MESASHADERBUILDDIR)unshared/program.o
#else
 MESA_SHADER_UOBJS = $(MESA_SHADER_OBJS)
#endif

  MESA_SHADER_DOBJS =	$(MESASHADERBUILDDIR)debugger/arbprogparse.o \
			$(MESASHADERBUILDDIR)debugger/arbprogram.o \
			$(MESASHADERBUILDDIR)debugger/arbfragparse.o \
			$(MESASHADERBUILDDIR)debugger/arbvertparse.o \
			$(MESASHADERBUILDDIR)debugger/grammar_mesa.o \
			$(MESASHADERBUILDDIR)debugger/nvprogram.o \
			$(MESASHADERBUILDDIR)debugger/nvfragparse.o \
			$(MESASHADERBUILDDIR)debugger/nvvertexec.o \
			$(MESASHADERBUILDDIR)debugger/nvvertparse.o \
			$(MESASHADERBUILDDIR)debugger/program.o

  MESA_SHADER_POBJS =	$(MESASHADERBUILDDIR)profiled/arbprogparse.o \
			$(MESASHADERBUILDDIR)profiled/arbprogram.o \
			$(MESASHADERBUILDDIR)profiled/arbfragparse.o \
			$(MESASHADERBUILDDIR)profiled/arbvertparse.o \
			$(MESASHADERBUILDDIR)profiled/grammar_mesa.o \
			$(MESASHADERBUILDDIR)profiled/nvprogram.o \
			$(MESASHADERBUILDDIR)profiled/nvfragparse.o \
			$(MESASHADERBUILDDIR)profiled/nvvertexec.o \
			$(MESASHADERBUILDDIR)profiled/nvvertparse.o \
			$(MESASHADERBUILDDIR)profiled/program.o
