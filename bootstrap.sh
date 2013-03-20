#! /usr/bin/env bash

set -eu

if [ -x "gnulib/gnulib-tool" ]; then
  gnulibtool=gnulib/gnulib-tool
else
  gnulibtool=gnulib-tool
fi

gnulib_modules=( git-version-gen gitlog-to-changelog gnupload signal-h
                 maintainer-makefile announce-gen getopt-gnu )

$gnulibtool --m4-base build-aux --source-base libgnu --import "${gnulib_modules[@]}"

sed -e 's/^sc_error_message_uppercase/disabled_sc_error_message_uppercase/' \
    -e 's/^sc_prohibit_path_max_allocation/disabled_sc_prohibit_path_max_allocation/' \
    -e 's/^sc_unmarked_diagnostics/disabled_sc_unmarked_diagnostics/' \
    <maint.mk >maint.mk-new && mv maint.mk-new maint.mk

build-aux/gitlog-to-changelog >>ChangeLog

autoreconf --install -Wall
