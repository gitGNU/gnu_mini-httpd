#
# Build mini-httpd
#

sbin_PROGRAMS   = httpd
httpd_SOURCES   = main.cc log.cc config.cc HTTPParser.cc                \
                  rh-construction.cc rh-copy-file.cc                    \
                  rh-standard-replies.cc rh-log-access.cc               \
                  rh-read-request-header.cc rh-read-request-line.cc     \
                  rh-setup-reply.cc rh-terminate.cc rh-flush-buffer.cc  \
                  rh-io-callbacks.cc rh-read-request-body.cc

httpd_CPPFLAGS  = -DPREFIX=\"${prefix}\"
httpd_LDADD     = ${LIBOBJS}

noinst_HEADERS  = HTTPParser.hh HTTPRequest.hh RequestHandler.hh        \
                  config.hh escape-html-specials.hh log.hh              \
                  resetable-variable.hh search-and-replace.hh           \
                  tcp-listener.hh urldecode.hh timestamp-to-string.hh   \
                  libscheduler/pollvector.hh libscheduler/scheduler.hh  \
                  system-error/system-error.hh ScopeGuard/ScopeGuard.hh

httpd.8:	man-page.sgml
	${docbook2man} $<
	@rm -f manpage.links manpage.refs

man_MANS 	= httpd.8
EXTRA_DIST      = ${man_MANS} README man-page.sgml
CLEANFILES	= manpage.links manpage.refs

#
# Maintainer Targets
#

init-src:
	@rm -f MT/monotonerc
	@ln -s ../.monotonerc MT/monotonerc
	monotone --branch=to.cryp.ScopeGuard co ScopeGuard
	monotone --branch=to.cryp.system-error co system-error
	monotone --branch=to.cryp.libscheduler co libscheduler
	monotone --branch=to.cryp.libgetopt co libgetopt
	aclocal-1.7
	autoconf
	automake -a
