#
# Build httpd
#
include autoconf.mk

DESTDIR		=

OPTIMFLAGS     +=
WARNFLAGS      +=
DEFS	       +=
CPPFLAGS       += -I.
CXXFLAGS       +=
LDFLAGS	       +=

OBJS	       += main.o log.o config.o HTTPParser.o rh-construction.o \
		  rh-copy-file.o rh-standard-replies.o rh-log-access.o \
		  rh-read-request-header.o rh-read-request-line.o      \
		  rh-setup-reply.o rh-terminate.o rh-flush-buffer.o    \
		  rh-io-callbacks.o rh-read-request-body.o
LIBOBJS	       +=
LIBS	       +=

.SUFFIXES:
.SUFFIXES:	.cc .o

.cc.o:
	$(CXX) $(CPPFLAGS) $(DEFS) $(CXXFLAGS) $(WARNFLAGS) $(OPTIMFLAGS) -c $< -o $@

all:		httpd

test:		test.o HTTPParser.o log.o
	$(CXX) $(LDFLAGS) $^ -o $@

httpd:		$(OBJS) $(LIBOBJS)
	$(CXX) $(LDFLAGS) $(OBJS) $(LIBOBJS) $(LIBS) -o $@

run:		httpd
	su -c "`pwd`/httpd -d -p 8080 -r `pwd` -s peti/1.0 -u 1000 -H `hostname`"

config.o:	config.cc
	$(CXX) -DPREFIX=\"$(prefix)\" $(CPPFLAGS) $(DEFS) $(CXXFLAGS) $(WARNFLAGS) $(OPTIMFLAGS) -c $< -o $@

config.o main.o:	version.h

version.h:	VERSION
	@echo  >$@ '/* Generated automatically; do not edit! */'
	@echo >>$@ ''
	@echo >>$@ '#ifndef VERSION_H'
	@echo >>$@ '#define VERSION_H'
	@echo >>$@ ''
	@sed <$< >>$@ -e 's/^/#define VERSION "/' -e 's/$$/"/'
	@echo >>$@ ''
	@echo >>$@ '#endif'
	@echo Built version.h file.

configure:	configure.ac
	autoconf

autoconf.mk:	autoconf.mk.in configure
	${HTTPD_AC_VARS} $(SHELL) configure ${HTTPD_AC_ARGS}

install:	httpd
	$(MKDIR) -p $(DESTDIR)$(bindir)
	$(INSTALL) -c -m 555 httpd $(DESTDIR)$(bindir)/

clean::
	@find . -name '*.rpo' -exec rm -f {} \;
	@find . -name '*.a'   -exec rm -f {} \;
	@find . -name '*.o'   -exec rm -f {} \;
	@find . -name '*.so'  -exec rm -f {} \;
	@rm -f httpd test
	@echo All dependent files have been removed.

distclean:: clean
	@rm -f autoconf.mk config.log config.status

realclean:: distclean
	@rm -f configure version.h

depend::
	makedepend -Y -fGNUmakefile '-s# Dependencies' `find . -type f -name '*.cc' ` 2>/dev/null
	@sed <GNUmakefile >GNUmakefile.bak -e 's/\.\///g'
	@mv GNUmakefile.bak GNUmakefile

# Dependencies

HTTPParser.o: HTTPParser.hh HTTPRequest.hh resetable-variable.hh
config.o: log.hh config.hh resetable-variable.hh
log.o: log.hh config.hh resetable-variable.hh
main.o: tcp-listener.hh ScopeGuard/ScopeGuard.hh
main.o: system-error/system-error.hh libscheduler/scheduler.hh
main.o: libscheduler/pollvector.hh RequestHandler.hh HTTPRequest.hh
main.o: resetable-variable.hh log.hh config.hh
rh-construction.o: ScopeGuard/ScopeGuard.hh system-error/system-error.hh
rh-construction.o: RequestHandler.hh libscheduler/scheduler.hh
rh-construction.o: libscheduler/pollvector.hh HTTPRequest.hh
rh-construction.o: resetable-variable.hh config.hh log.hh
rh-copy-file.o: system-error/system-error.hh RequestHandler.hh
rh-copy-file.o: libscheduler/scheduler.hh libscheduler/pollvector.hh
rh-copy-file.o: HTTPRequest.hh resetable-variable.hh log.hh
rh-flush-buffer.o: RequestHandler.hh libscheduler/scheduler.hh
rh-flush-buffer.o: libscheduler/pollvector.hh HTTPRequest.hh
rh-flush-buffer.o: resetable-variable.hh log.hh
rh-io-callbacks.o: system-error/system-error.hh RequestHandler.hh
rh-io-callbacks.o: libscheduler/scheduler.hh libscheduler/pollvector.hh
rh-io-callbacks.o: HTTPRequest.hh resetable-variable.hh config.hh log.hh
rh-log-access.o: system-error/system-error.hh RequestHandler.hh
rh-log-access.o: libscheduler/scheduler.hh libscheduler/pollvector.hh
rh-log-access.o: HTTPRequest.hh resetable-variable.hh
rh-log-access.o: timestamp-to-string.hh search-and-replace.hh config.hh
rh-log-access.o: log.hh
rh-read-request-body.o: RequestHandler.hh libscheduler/scheduler.hh
rh-read-request-body.o: libscheduler/pollvector.hh HTTPRequest.hh
rh-read-request-body.o: resetable-variable.hh log.hh
rh-read-request-header.o: RequestHandler.hh libscheduler/scheduler.hh
rh-read-request-header.o: libscheduler/pollvector.hh HTTPRequest.hh
rh-read-request-header.o: resetable-variable.hh HTTPParser.hh log.hh
rh-read-request-line.o: RequestHandler.hh libscheduler/scheduler.hh
rh-read-request-line.o: libscheduler/pollvector.hh HTTPRequest.hh
rh-read-request-line.o: resetable-variable.hh HTTPParser.hh urldecode.hh
rh-read-request-line.o: log.hh
rh-setup-reply.o: system-error/system-error.hh HTTPParser.hh HTTPRequest.hh
rh-setup-reply.o: resetable-variable.hh RequestHandler.hh
rh-setup-reply.o: libscheduler/scheduler.hh libscheduler/pollvector.hh
rh-setup-reply.o: timestamp-to-string.hh escape-html-specials.hh
rh-setup-reply.o: urldecode.hh config.hh log.hh
rh-standard-replies.o: RequestHandler.hh libscheduler/scheduler.hh
rh-standard-replies.o: libscheduler/pollvector.hh HTTPRequest.hh
rh-standard-replies.o: resetable-variable.hh HTTPParser.hh
rh-standard-replies.o: escape-html-specials.hh timestamp-to-string.hh
rh-standard-replies.o: config.hh log.hh
rh-terminate.o: RequestHandler.hh libscheduler/scheduler.hh
rh-terminate.o: libscheduler/pollvector.hh HTTPRequest.hh
rh-terminate.o: resetable-variable.hh log.hh
test.o: HTTPParser.hh HTTPRequest.hh resetable-variable.hh log.hh
libscheduler/test.o: libscheduler/scheduler.hh libscheduler/pollvector.hh
