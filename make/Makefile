# Makefile to build the site

HTMLEX = ../htmlex -k

include Makefile.list

.PHONY: all clean depend

all: $(HTML_FILES)

clean:
	-rm -f Makefile.deps $(HTML_FILES)

depend:
	$(HTMLEX) -d -c $(HTEX_FILES) -o $(HTML_FILES) > Makefile.deps

out/%.html: src/%.htex
	$(HTMLEX) $< > $@
#	$(HTMLEX) $< | ../htmlex-filter.pl > $@

-include Makefile.deps
