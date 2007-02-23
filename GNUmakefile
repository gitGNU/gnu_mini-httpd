TOOLSET  = gcc-3.4.6
#TOOLSET = intel-linux-9.1

VARIANT  = debug
#VARIANT = release

DISTDIR  = $(shell pwd)/dist
HTTPD    = bin/${TOOLSET}/${VARIANT}/mini-httpd

.PHONY: all run clean

all:
	bjam --without-mpi toolset="${TOOLSET}" variant="${VARIANT}"

run:		all ${DISTDIR}
	${HTTPD} -d -D -p 8080 -l ${DISTDIR} --document-root ${DISTDIR}

clean:
	rm -rf bin

${DISTDIR}:
	mkdir $@
