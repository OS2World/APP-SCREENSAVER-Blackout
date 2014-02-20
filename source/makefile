#==========================================================================
#This file is part of the Blackout screen saver.           v1.00 1996-06-29
#Copyright (C) 1995, 1996 by Staffan Ulfberg.  See README file for details.
#==========================================================================
CC=gcc
CFLAGS=-s -Wall -Wno-parentheses -Zmt

OBJS=grey.o blackout.o black.o 16bit.o
DEPEND=$(OBJS:.o=.d)

all:		blackout.exe blackdll.dll black.exe

%.d: %.c
	$(CC) -MM $< | sed 's/$*.o[ :]*/$@ &/g' > $@

include $(DEPEND)

%.exe:
	$(CC) -Zmt -s  $^ -lgraph

%.dll:
	$(CC) -Zdll -Zso -Zsys -Zomf -O $^


blackout.res:	blackout.rc blackout.h blackout.ico
	rc -r blackout

blackout.exe:	blackout.o blackout.def blackout.res grey.o 16bit.o 

blackdll.dll:	blackdll.c blackdll.def

black.exe:	black.o black.def 16bit.o


# ------------- Make zipfiles ---------------------------------

%.zip:
	zip -9 $@ $^

blackxx.zip:	README COPYING blackout.exe black.exe \
		blackdll.dll source.zip

source.zip:	Makefile blackout.def blackout.rc blackout.ico \
		blackout.c blackout.h grey.c grey.h blackdll.def \
		blackdll.c black.c black.def 16bit.c 16bit.h vga.h

zip:	blackxx.zip source.zip

