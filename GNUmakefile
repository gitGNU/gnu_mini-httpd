ROOT   = $(shell pwd)
HTDOCS = ${ROOT}/dist
LOGS   = ${ROOT}/dist
HTTPD  = bin/gcc-3.4.6/debug/mini-httpd

run:		${LOGS} ${HTDOCS}
	${HTTPD} -d -D -p 8080 -l ${LOGS} --document-root ${HTDOCS}

${LOGS} ${HTDOCS}:
	mkdir $@
