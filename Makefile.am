# Build mini-httpd
#
# Copyright (c) 2001-2010 Peter Simons <simons@cryp.to>
#
# This program is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation, either version 3 of the License, or (at your option) any later
# version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program. If not, see <http://www.gnu.org/licenses/>.

ACLOCAL_AMFLAGS = -I build-aux
SUBDIRS         = libgnu

sbin_PROGRAMS   = httpd
httpd_SOURCES   = main.cc log.cc config.cc HTTPParser.cc                \
                  rh-construction.cc rh-copy-file.cc                    \
                  rh-standard-replies.cc rh-log-access.cc               \
                  rh-read-request-header.cc rh-read-request-line.cc     \
                  rh-setup-reply.cc rh-terminate.cc rh-flush-buffer.cc  \
                  rh-io-callbacks.cc rh-read-request-body.cc

httpd_CPPFLAGS  = -DPREFIX=\"$(prefix)\" -Ilibgnu
httpd_LDADD     = libgnu/libgnu.a

noinst_HEADERS  = HTTPParser.hh HTTPRequest.hh RequestHandler.hh        \
                  config.hh escape-html-specials.hh log.hh              \
                  resetable-variable.hh search-and-replace.hh           \
                  tcp-listener.hh urldecode.hh timestamp-to-string.hh   \
                  libscheduler/pollvector.hh libscheduler/scheduler.hh  \
                  system-error.hh

man_MANS        = httpd.8
EXTRA_DIST      = $(man_MANS) README man-page.xml build-aux/gnulib-cache.m4

httpd.8:	man-page.xml
	xmlto man $<
