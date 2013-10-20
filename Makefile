# Makefile for htmlex

PACKAGE = htmlex

GCC = gcc

CFLAGS = -W -Wall -Wno-unused-parameter
LFLAGS =

ifdef DEBUG
CFLAGS += -s
LFLAGS += -s
else
CFLAGS += -g
LFLAGS += -g
endif

ifeq ($(OS),Windows_NT)
LFLAGS += -mconsole
EXE = .exe
endif

FILES = htmlex macros stream tags

all: $(PACKAGE)$(EXE)

%.o: %.c
	$(GCC) $(CFLAGS) -o $@ -c $<

$(PACKAGE)$(EXE): $(addsuffix .o, $(FILES))
	$(GCC) $(LFLAGS) -o $@ $^

clean:
	-rm -fv $(addsuffix .o, $(FILES)) Makefile.dep

distclean: clean
	-rm -fv $(PACKAGE)$(EXE)

depend:
	$(GCC) -MM $(addsuffix .c, $(FILES)) > Makefile.dep

-include Makefile.dep
