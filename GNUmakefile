TOOLSET  = gcc-3.4.6
#TOOLSET = intel-linux-9.1

VARIANT  = debug
#VARIANT = release

DISTDIR  = $(shell pwd)/dist
#HTTPD    = bin/${TOOLSET}/${VARIANT}/threading-multi/mini-httpd
HTTPD   = bin/${TOOLSET}/${VARIANT}/micro-httpd

.PHONY: all run clean

all:
	bjam --without-mpi toolset="${TOOLSET}" variant="${VARIANT}"

run:		all ${DISTDIR}
	${HTTPD} --no-detach --listen=127.0.0.1:8888 --log-dir ${DISTDIR} --document-root ${DISTDIR} 8080 ::1:8081

clean:
	rm -rf bin

${DISTDIR}:
	mkdir $@
