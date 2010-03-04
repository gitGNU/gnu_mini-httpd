#! /bin/sh

set -eu

if [ -x "gnulib/gnulib-tool" ]; then
  gnulibtool=gnulib/gnulib-tool
else
  gnulibtool=gnulib-tool
fi

gnulib_modules=( git-version-gen gitlog-to-changelog gnupload
		 maintainer-makefile announce-gen getopt-gnu )

$gnulibtool --m4-base build-aux --source-base libgnu --import "${gnulib_modules[@]}"

build-aux/gitlog-to-changelog >>ChangeLog

autoreconf --install -Wall
