# Makefile for htmlex

PACKAGE = htmlex

ifdef DJDIR
  EXE = .exe
else
  EXE =
endif

GCC = gcc

CFLAGS = -s -W -Wall -Wno-unused
LFLAGS = -s

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
