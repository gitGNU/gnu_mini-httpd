#
# Build the httpd documentation
#

.SUFFIXES:

%.tex : %.sgml
	sgmltools -b jadetex $<

%.html : %.sgml
	sgmltools -b onehtml $<

%.txt : %.sgml
	sgmltools -b w3m $<

%.dvi : %.tex
	jadetex \\nonstopmode\\input $<; \
	while grep -i 'Rerun' $*.log; do \
	     jadetex \\nonstopmode\\input $<; \
	done

%.ps : %.dvi
	dvips $< -o $@

%.pdf : %.dvi
	dvipdf $< $@

.PHONY: all clean distclean realclean

all:	httpd.html httpd.pdf man-page.txt readme.txt

man-page.html:	synopsis.inc description.inc cmdloptions.inc configfile.inc \
		license.inc

httpd.html httpd.tex: license.inc building.inc synopsis.inc cmdloptions.inc
httpd.html httpd.tex: configfile.inc

readme.txt:	readme.sgml building.inc ../VERSION

man-page.txt:	man-page.html
	lynx -width=80 -nolist -dump $< >$@

defconfig.inc:	../config-sample Makefile
	sed <$< >$@ -e 's/&/\&amp;/g' \
		    -e 's/</\&lt;/g' \
		    -e 's/>/\&gt;/g' \
		    -e 's#-#<markup>-</markup>#g' \
		    -e '/^#/d'	     \
		    -e '/^$$/d'

defreqmail.inc:../reqmail.template-sample Makefile
	sed <$< >$@ -e 's/</\&lt;/g' \
		    -e 's/>/\&gt;/g' \
		    -e 's/&/\&amp;/g' \
		    -e 's#-#<markup>-</markup>#g' \
		    -e 's#username@example.com#<replaceable>&</replaceable>#g' \
		    -e 's#Real Name#<replaceable>&</replaceable>#g'

clean distclean:
	@rm -f *.aux *.dvi *.log *.toc
	@rm -f *.css *.fot *.ps *.xml
	@rm -f *.tex defconfig.inc defreqmail.inc

realclean:	clean
	@rm -f *.txt *.pdf *.html

