TOOLSET  = gcc-3.4.6
#TOOLSET = intel-linux-9.1

VARIANT  = debug
#VARIANT = release

DISTDIR  = $(shell pwd)/dist
HTTPD    = bin/${TOOLSET}/${VARIANT}/threading-multi/mini-httpd

.PHONY: all run clean

all:
	bjam --without-mpi toolset="${TOOLSET}" variant="${VARIANT}"

run:		all ${DISTDIR}
	${HTTPD} --no-detach --listen=127.1:8888 --port 8080 --log-dir ${DISTDIR} --document-root ${DISTDIR} --default-hostname ""

clean:
	rm -rf bin

${DISTDIR}:
	mkdir $@
