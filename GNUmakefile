#
# Build httpd
#
include autoconf.mk

DESTDIR		=

OPTIMFLAGS     +=
WARNFLAGS      +=
DEFS	       += -DSPIRIT_DEBUG
CPPFLAGS       +=
CXXFLAGS       +=
LDFLAGS	       +=

OBJS	       += main.o log.o config.o rh-construction.o rh-misc.o \
		  rh-process-input.o rh-readable.o rh-timeouts.o \
		  rh-writable.o rh-errors.o HTTPParser.o
LIBOBJS	       += libscheduler/scheduler.o
LIBS	       +=

.SUFFIXES:
.SUFFIXES:	.cc .o

.cc.o:
	$(CXX) $(CPPFLAGS) $(DEFS) $(CXXFLAGS) $(WARNFLAGS) $(OPTIMFLAGS) -c $< -o $@

all:		httpd http-parser

httpd:		$(OBJS) $(LIBOBJS)
	$(CXX) $(LDFLAGS) $(OBJS) $(LIBOBJS) $(LIBS) -o $@

http-parser:	http-parser.o HTTPParser.o
	$(CXX) $(LDFLAGS) $^ $(LIBS) -o $@

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
	@rm -f httpd http-parser
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

config.o: config.hh log.hh
log.o: log.hh
main.o: tcp-listener.hh ScopeGuard/ScopeGuard.hh
main.o: system-error/system-error.hh libscheduler/scheduler.hh
main.o: libscheduler/pollvector.hh log.hh request-handler.hh
main.o: RegExp/RegExp.hh config.hh
rh-construction.o: ScopeGuard/ScopeGuard.hh system-error/system-error.hh
rh-construction.o: request-handler.hh libscheduler/scheduler.hh
rh-construction.o: libscheduler/pollvector.hh RegExp/RegExp.hh log.hh
rh-construction.o: config.hh
rh-errors.o: request-handler.hh libscheduler/scheduler.hh
rh-errors.o: libscheduler/pollvector.hh RegExp/RegExp.hh log.hh config.hh
rh-misc.o: system-error/system-error.hh request-handler.hh
rh-misc.o: libscheduler/scheduler.hh libscheduler/pollvector.hh
rh-misc.o: RegExp/RegExp.hh log.hh config.hh
rh-process-input.o: request-handler.hh libscheduler/scheduler.hh
rh-process-input.o: libscheduler/pollvector.hh RegExp/RegExp.hh log.hh
rh-readable.o: request-handler.hh libscheduler/scheduler.hh
rh-readable.o: libscheduler/pollvector.hh RegExp/RegExp.hh log.hh config.hh
rh-readable.o: urldecode.hh
rh-timeouts.o: request-handler.hh libscheduler/scheduler.hh
rh-timeouts.o: libscheduler/pollvector.hh RegExp/RegExp.hh log.hh
rh-writable.o: request-handler.hh libscheduler/scheduler.hh
rh-writable.o: libscheduler/pollvector.hh RegExp/RegExp.hh log.hh
libscheduler/scheduler.o: libscheduler/scheduler.hh
libscheduler/scheduler.o: libscheduler/pollvector.hh
libscheduler/test.o: libscheduler/scheduler.hh libscheduler/pollvector.hh
RegExp/test.o: RegExp/RegExp.hh
http-parser.o: system-error/system-error.hh HTTPParser.hh
HTTPParser.o: HTTPParser.hh
