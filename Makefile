# Makefile for htmlex

PACKAGE = htmlex
VERSION = 0.2

ifdef DJDIR
  EXE = .exe
  ZIP = .zip
  TAR = tar -9
else
  EXE =
  ZIP = .tar.gz
  TAR = tar chozf
endif

CFLAGS = -s -W -Wall -Wno-unused
LFLAGS = -s

FILES = htmlex macros stream tags

DOC_DISTFILES = \
	cambios.txt \
	changes.txt \
	leame.txt \
	license.txt \
	news.txt \
	noticias.txt \
	readme.txt \
	$(wildcard examples/*.htex)

SRC_DISTFILES = \
	$(addsuffix .c, $(FILES)) \
	$(addsuffix .h, $(FILES)) \
	Makefile

BIN_DISTFILES = \
	$(PACKAGE)$(EXE)

distdir = $(PACKAGE)-$(VERSION)

all: $(PACKAGE)$(EXE)

%.o: %.c
	gcc $(CFLAGS) -o $@ -c $<

$(PACKAGE)$(EXE): $(addsuffix .o, $(FILES))
	gcc $(LFLAGS) -o $@ $^

clean:
	-rm -fv $(addsuffix .o, $(FILES)) Makefile.dep

distclean: clean
	-rm -fv $(PACKAGE)$(EXE)

depend:
	gcc -MM $(addsuffix .c, $(FILES)) > Makefile.dep

dist: dist-src dist-bin

dist-src:
	mkdir $(distdir)
	-chmod 777 $(distdir)
	cp -Ppr $(DOC_DISTFILES) $(SRC_DISTFILES) $(distdir)
	$(TAR) $(distdir)$(ZIP) $(distdir)
	-rm -rf $(distdir)

dist-bin:
	make
	mkdir $(distdir)
	-chmod 777 $(distdir)
	cp -Ppr $(DOC_DISTFILES) $(BIN_DISTFILES) $(distdir)
	$(TAR) $(distdir)-bin$(ZIP) $(distdir)
	-rm -rf $(distdir)

dtou:
	-dtou $(DOC_DISTFILES) $(SRC_DISTFILES)

utod:
	-utod $(DOC_DISTFILES) $(SRC_DISTFILES)

-include Makefile.dep

