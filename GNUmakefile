TOOLSET  = gcc-3.4.6
#TOOLSET = intel-linux-9.1

VARIANT  = debug
#VARIANT = release

DISTDIR  = $(shell pwd)/dist
HTTPD    = bin/${TOOLSET}/${VARIANT}/threading-multi/httpd
#HTTPD   = bin/${TOOLSET}/${VARIANT}/httpd

.PHONY: all run clean

all:
	bjam --without-mpi toolset="${TOOLSET}" variant="${VARIANT}"

run:		all ${DISTDIR}
	${HTTPD} --no-detach --log-dir ${DISTDIR} --document-root ${DISTDIR} 8080

clean:
	rm -rf bin

${DISTDIR}:
	mkdir $@

redate:
	redate main.cpp Doxyfile
