#
# Build httpd
#
include autoconf.mk

DESTDIR		=

OPTIMFLAGS     +=
WARNFLAGS      +=
DEFS	       +=
CPPFLAGS       +=
CFLAGS	       +=
CXXFLAGS       +=
LDFLAGS	       +=

OBJS	       += main.o log.o config.o rh-construction.o rh-misc.o \
		  rh-process-input.o rh-readable.o rh-timeouts.o \
		  rh-writable.o rh-errors.o file-cache.o
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

config.o: config.hh log.hh
file-cache.o: system-error/system-error.hh log.hh file-cache.hh
file-cache.o: refcount-auto-ptr/refcount-auto-ptr.hh
log.o: log.hh
main.o: tcp-listener.hh system-error/system-error.hh
main.o: libscheduler/scheduler.hh libscheduler/pollvector.hh log.hh
main.o: request-handler.hh RegExp/RegExp.hh file-cache.hh
main.o: refcount-auto-ptr/refcount-auto-ptr.hh config.hh
rh-construction.o: system-error/system-error.hh request-handler.hh
rh-construction.o: libscheduler/scheduler.hh libscheduler/pollvector.hh
rh-construction.o: RegExp/RegExp.hh log.hh file-cache.hh
rh-construction.o: refcount-auto-ptr/refcount-auto-ptr.hh config.hh
rh-errors.o: request-handler.hh libscheduler/scheduler.hh
rh-errors.o: libscheduler/pollvector.hh RegExp/RegExp.hh log.hh
rh-errors.o: file-cache.hh refcount-auto-ptr/refcount-auto-ptr.hh config.hh
rh-misc.o: system-error/system-error.hh request-handler.hh
rh-misc.o: libscheduler/scheduler.hh libscheduler/pollvector.hh
rh-misc.o: RegExp/RegExp.hh log.hh file-cache.hh
rh-misc.o: refcount-auto-ptr/refcount-auto-ptr.hh config.hh
rh-process-input.o: request-handler.hh libscheduler/scheduler.hh
rh-process-input.o: libscheduler/pollvector.hh RegExp/RegExp.hh log.hh
rh-process-input.o: file-cache.hh refcount-auto-ptr/refcount-auto-ptr.hh
rh-readable.o: request-handler.hh libscheduler/scheduler.hh
rh-readable.o: libscheduler/pollvector.hh RegExp/RegExp.hh log.hh
rh-readable.o: file-cache.hh refcount-auto-ptr/refcount-auto-ptr.hh
rh-readable.o: config.hh urldecode.hh
rh-timeouts.o: request-handler.hh libscheduler/scheduler.hh
rh-timeouts.o: libscheduler/pollvector.hh RegExp/RegExp.hh log.hh
rh-timeouts.o: file-cache.hh refcount-auto-ptr/refcount-auto-ptr.hh
rh-writable.o: request-handler.hh libscheduler/scheduler.hh
rh-writable.o: libscheduler/pollvector.hh RegExp/RegExp.hh log.hh
rh-writable.o: file-cache.hh refcount-auto-ptr/refcount-auto-ptr.hh
libscheduler/scheduler.o: libscheduler/scheduler.hh
libscheduler/scheduler.o: libscheduler/pollvector.hh
libscheduler/test.o: libscheduler/scheduler.hh libscheduler/pollvector.hh
RegExp/test.o: RegExp/RegExp.hh
refcount-auto-ptr/test.o: refcount-auto-ptr/refcount-auto-ptr.hh
