#
# Build the httpd documentation
#
DOCBOOKBASE	= /usr/share/sgml/docbook/dsssl-stylesheets-1.78

.SUFFIXES:

%.tex : %.sgml
	openjade -t tex -d peti.dsl#print -D $(DOCBOOKBASE)/print $<

%.html : %.sgml
	openjade -t sgml -d peti.dsl#html -D $(DOCBOOKBASE)/html $< >$@

%.dvi : %.tex
	jadetex \\nonstopmode\\input $<; \
	jadetex \\nonstopmode\\input $<; \
	jadetex \\nonstopmode\\input $<; \

%.ps : %.dvi
	dvips $< -o $@

%.pdf : %.dvi
	dvipdf $< $@

.PHONY: all clean distclean realclean

all:	httpd.html httpd.pdf httpd.8 readme.txt

httpd.html httpd.tex: license.inc building.inc synopsis.inc cmdloptions.inc ../VERSION
httpd.html httpd.tex: setup.inc peti.dsl

readme.txt:	readme.sgml building.inc setup.inc ../VERSION peti.dsl
	docbook2txt readme.sgml

httpd.8:  man-page.sgml synopsis.inc description.inc cmdloptions.inc license.inc
	docbook2man man-page.sgml

clean distclean:
	@rm -f *.aux *.dvi *.log *.toc
	@rm -f *.css *.fot *.ps *.xml
	@rm -f *.tex defconfig.inc defreqmail.inc

realclean:	clean
	@rm -f *.txt *.pdf *.html *.8

