#
# Build the httpd documentation
#
DOCBOOKBASE	= /usr/local/sgml/dsssl/docbook

.SUFFIXES:

%.tex : %.sgml
	openjade -t tex -d peti.dsl#print -D $(DOCBOOKBASE)/print $<

%.html : %.sgml
	openjade -t sgml -d peti.dsl#html -D $(DOCBOOKBASE)/html $< >$@

%.txt : %.sgml
	sgmltools -b w3m $<

%.dvi : %.tex
	jadetex \\nonstopmode\\input $<; \
	jadetex \\nonstopmode\\input $<; \
	jadetex \\nonstopmode\\input $<; \

%.ps : %.dvi
	dvips $< -o $@

%.pdf : %.dvi
	dvipdf $< $@

.PHONY: all clean distclean realclean

all:	httpd.html httpd.pdf man-page.txt readme.txt

man-page.html:	synopsis.inc description.inc cmdloptions.inc license.inc

httpd.html httpd.tex: license.inc building.inc synopsis.inc cmdloptions.inc ../VERSION
httpd.html httpd.tex: setup.inc peti.dsl

readme.txt:	readme.sgml building.inc setup.inc ../VERSION peti.dsl

man-page.html:	man-page.sgml synopsis.inc description.inc cmdloptions.inc license.inc

man-page.txt:  man-page.html
	lynx -width=80 -nolist -dump $< >$@

clean distclean:
	@rm -f *.aux *.dvi *.log *.toc
	@rm -f *.css *.fot *.ps *.xml
	@rm -f *.tex defconfig.inc defreqmail.inc

realclean:	clean
	@rm -f *.txt *.pdf *.html

