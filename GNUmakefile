#
# Build httpd
#
include autoconf.mk

DESTDIR		=

OPTIMFLAGS     +=
WARNFLAGS      +=
DEFS	       += -DDEBUG
CPPFLAGS       +=
CXXFLAGS       +=
LDFLAGS	       +=

OBJS	       += main.o log.o config.o HTTPParser.o rh-construction.o \
		  rh-copy-file.o rh-errors.o rh-get-request-body.o     \
		  rh-get-request-header.o rh-get-request-line.o        \
		  rh-readable.o rh-setup-reply.o rh-terminate.o        \
		  rh-timeouts.o rh-writable.o rh-write-remaining-data.o
LIBOBJS	       += libscheduler/scheduler.o
LIBS	       +=

.SUFFIXES:
.SUFFIXES:	.cc .o

.cc.o:
	$(CXX) $(CPPFLAGS) $(DEFS) $(CXXFLAGS) $(WARNFLAGS) $(OPTIMFLAGS) -c $< -o $@

all:		httpd

httpd:		$(OBJS) $(LIBOBJS)
	$(CXX) $(LDFLAGS) $(OBJS) $(LIBOBJS) $(LIBS) -o $@

config.o:	config.cc
	$(CXX) -DDOCUMENT_ROOT=\"/home/simons/projects/httpd\" $(CPPFLAGS) $(DEFS) $(CXXFLAGS) $(WARNFLAGS) $(OPTIMFLAGS) -c $< -o $@

configure:	configure.ac
	autoconf

autoconf.mk:	autoconf.mk.in configure GNUmakefile
	${HTTPD_AC_VARS} $(SHELL) configure ${HTTPD_AC_ARGS}

install:	httpd
	$(MKDIR) -p $(DESTDIR)$(bindir)
	$(INSTALL) -c -m 555 httpd $(DESTDIR)$(bindir)/

clean::
	@find . -name '*.rpo' -exec rm -f {} \;
	@find . -name '*.a'   -exec rm -f {} \;
	@find . -name '*.o'   -exec rm -f {} \;
	@find . -name '*.so'  -exec rm -f {} \;
	@rm -f httpd
	@echo All dependent files have been removed.

distclean:: clean
	@rm -f autoconf.mk config.log config.status

realclean:: distclean
	@rm -f configure

depend::
	makedepend -Y -fGNUmakefile '-s# Dependencies' `find . -type f -name '*.c*' ` 2>/dev/null
	@sed <GNUmakefile >GNUmakefile.bak -e 's/\.\///g'
	@mv GNUmakefile.bak GNUmakefile

# Dependencies

HTTPParser.o: HTTPParser.hh
config.o: config.hh log.hh
rh-copy-file.o: system-error/system-error.hh request-handler.hh
rh-copy-file.o: libscheduler/scheduler.hh libscheduler/pollvector.hh log.hh
log.o: log.hh
main.o: tcp-listener.hh ScopeGuard/ScopeGuard.hh
main.o: system-error/system-error.hh libscheduler/scheduler.hh
main.o: libscheduler/pollvector.hh log.hh request-handler.hh config.hh
rh-construction.o: system-error/system-error.hh request-handler.hh
rh-construction.o: libscheduler/scheduler.hh libscheduler/pollvector.hh
rh-construction.o: config.hh log.hh
rh-errors.o: request-handler.hh libscheduler/scheduler.hh
rh-errors.o: libscheduler/pollvector.hh config.hh log.hh
rh-setup-reply.o: request-handler.hh libscheduler/scheduler.hh
rh-setup-reply.o: libscheduler/pollvector.hh config.hh log.hh
rh-readable.o: system-error/system-error.hh request-handler.hh
rh-readable.o: libscheduler/scheduler.hh libscheduler/pollvector.hh log.hh
rh-timeouts.o: request-handler.hh libscheduler/scheduler.hh
rh-timeouts.o: libscheduler/pollvector.hh log.hh
rh-writable.o: system-error/system-error.hh request-handler.hh
rh-writable.o: libscheduler/scheduler.hh libscheduler/pollvector.hh log.hh
libscheduler/scheduler.o: libscheduler/scheduler.hh
libscheduler/scheduler.o: libscheduler/pollvector.hh
libscheduler/test.o: libscheduler/scheduler.hh libscheduler/pollvector.hh
rh-get-request-line.o: request-handler.hh libscheduler/scheduler.hh
rh-get-request-line.o: libscheduler/pollvector.hh HTTPParser.hh
rh-get-request-line.o: urldecode.hh log.hh
rh-get-request-header.o: request-handler.hh libscheduler/scheduler.hh
rh-get-request-header.o: libscheduler/pollvector.hh HTTPParser.hh log.hh
rh-get-request-body.o: request-handler.hh libscheduler/scheduler.hh
rh-get-request-body.o: libscheduler/pollvector.hh log.hh
rh-write-remaining-data.o: request-handler.hh libscheduler/scheduler.hh
rh-write-remaining-data.o: libscheduler/pollvector.hh log.hh
rh-terminate.o: request-handler.hh libscheduler/scheduler.hh
rh-terminate.o: libscheduler/pollvector.hh log.hh
