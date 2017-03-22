# Makefile for ieeeFormat 
#
# (template Makefile for simple targets)
#
# Core Library, $Id: Makefile,v 1.1 2006/03/07 04:51:24 exact Exp $

#=================================================
# Define variables
#=================================================

include ../Make.options

ifndef lev
	lev=3
endif

ifdef DEBUG
	XFLAGS = -DmyDEBUG
endif

p=ieeeFormat
p=ieee
p=coreEps
p=tDoubleValue
p=fpline
p=next
p=size
p=bigRand
p=frexp

#=================================================
# Define target files (put all your files here)
#=================================================
TARGETS= tDoubleValue

default: $(TARGETS)

MORETARGETS= ieeeFormat 

test: $(TARGETS)
	./tDoubleValue

moretest: $(MORETARGETS)
	./ieeeFormat

ieeeFormat: ieeeFormat.o

tDoubleValue: tDoubleValue.o

p $(p): $(p).o
	${CXX} $(LDFLAGS) $< $(CORE_LIB) -o $@

fpline13: fpline1 fpline3

fpline3 3: fpline3.o
	${CXX} $(LDFLAGS) $< -lcorex++$(VAR)_level3 $(CORE_LIB) -o fpline3
	
fpline3.o: fpline.cpp
	${CXX} -c $(CXXFLAGS) -DCORE_LEVEL=3 $(CORE_INC) $< -o fpline3.o

fpline1 1: fpline1.o
	${CXX} $(LDFLAGS) $< -lcorex++$(VAR)_level1 $(CORE_LIB) -o fpline1
	
fpline1.o: fpline.cpp
	${CXX} -c $(CXXFLAGS) -DCORE_LEVEL=1 $(CORE_INC) $< -o fpline1.o

fpline: fpline.o

line:
	./fpline > out

coreEps: coreEps.o
	${CXX} -c $(CXXFLAGS)  $(XFLAGS) $(CORE_INC) $< -o $@

bigRand: bigRand.o

frexp: frexp.o

tt: tt.o

#=================================================
# Rules
#=================================================
%: %.o
	${CXX} $(LDFLAGS) $< $(CORE_LIB) -o $@

.cpp.o:
	${CXX} -c $(CXXFLAGS) $(XFLAGS) $(CORE_INC) $< -o $@

#=================================================
# Clean object files
#=================================================
clean:
	-@test -z "*.o" || rm -f *.o

#=================================================
# Remove executable files
#=================================================
EXEPROGS=$(TARGETS:=$(EXETYPE)) $(MORETARGETS:=$(EXETYPE))

veryclean: clean
	-@test -z "$(EXEPROGS)" || rm -f $(EXEPROGS)
	-@test -z p$(EXETYPE) || rm -f p$(EXETYPE)

