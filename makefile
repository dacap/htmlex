# Makefile for HTMLex

VERSION = 01

ifndef UNIX
  EXE = .exe
else
  EXE =
endif
OBJ = .o

DOCDIR = docs
OBJDIR = obj
SRCDIR = src


TARGET_FILE = ./htmlex$(EXE)

SRC_FILES = expr.c htmlex.c macro.c memory.c stream.c string.c tag.c

OBJS_FILES = $(addprefix $(OBJDIR)/, $(addsuffix $(OBJ), $(basename $(SRC_FILES))))

HTML_FILES = $(addsuffix .html, $(basename $(wildcard $(DOCDIR)/*.htex)))

CLEAN_FILES = $(OBJS_FILES)

DISTCLEAN_FILES = $(TARGET_FILE) $(HTML_FILES) makefile.dep

SRC_PACK = \
	htmlex/COPYING \
	htmlex/*.es \
	htmlex/*.en \
	htmlex/*.sh \
	htmlex/makefile \
	htmlex/src/*.c \
	htmlex/src/*.h \
	htmlex/obj/*.txt \
	htmlex/docs/*.htex \
	htmlex/docs/*/*.htex \
	htmlex/examples/*.htex

BIN_PACK = \
	htmlex/htmlex$(EXE) \
	htmlex/COPYING \
	htmlex/*.es \
	htmlex/*.en \
	htmlex/docs/*.htex \
	htmlex/docs/*/*.htex \
	htmlex/examples/*.htex


CFLAGS = -W -Wall -Werror -O3
LFLAGS = -s


.PHONY: default all docs clean distclean depend help

.PRECIOUS: $(OBJDIR)/%$(OBJ)


default: $(TARGET_FILE) docs
	@echo Done!

all: default

docs: $(HTML_FILES)

cleandocs:

clean:
	-rm -fv $(CLEAN_FILES)

distclean: clean
	-rm -fv $(DISTCLEAN_FILES)

depend:
	-rm -fv makefile.dep
	make docs DEPEND=1
	cat makefile.dep | sed -e 's/^\(.*\)\.htex:/\1\.html:/g' > makefile.dep

pack:
	cd ../
	zip -9 htmlex$(VERSION)s.zip $(SRC_PACK)
	zip -9 htmlex$(VERSION)b.zip $(BIN_PACK)
	tar -zcf htmlex$(VERSION)s.tar.gz $(SRC_PACK)
	tar -zcf htmlex$(VERSION)b.tar.gz $(BIN_PACK)
	cd htmlex/

help:
	@echo "Usage:"
	@echo "  make [UNIX=1] [docs | clean | distclean | depend | pack | help]"

$(TARGET_FILE): $(OBJS_FILES)
	gcc -o $@ $^ $(LFLAGS)

$(OBJDIR)/%$(OBJ): $(SRCDIR)/%.c
	gcc $(CFLAGS) -o $@ -c $<

ifndef DEPEND

$(DOCDIR)/%.html: $(DOCDIR)/%.htex $(TARGET_FILE)
	$(TARGET_FILE) -I $(DOCDIR) $< > $@

-include makefile.dep

else

dummy_rule:

$(DOCDIR)/%.html: dummy_rule
	$(TARGET_FILE) -I $(DOCDIR) -M $(DOCDIR)/$(notdir $(basename $@)).htex >> makefile.dep

endif

