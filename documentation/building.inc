  <para>Compiling the software should be pretty straight forward; all you'll
have to do is the usual routine:</para>

  <screen>
./configure
gmake
gmake install</screen>

  <para>Be adviced, though, that the <command>configure</command> script
included in this distribution does not behave exactly like you will know from
other packages: The default <varname>$prefix</varname>, where &httpd; will be
installed, is not <filename>/usr/local</filename> but
<filename>/usr/local/httpd</filename>! This is for a very good reason. For
maximum security, &httpd; will run in a restricted sandbox, meaning, that it
will change the processes <filename>/</filename> directory to the default
<varname>$prefix</varname> directory provided here. Since no files required by
&httpd; may reside outside this directory then (logfiles and the actual web
documents, mostly), it is a very good idea to have all files related to the web
service installed in <filename>/usr/local/httpd</filename>.</para>

  <para>If you don't understand any of this, just trust me and and go ahead with
the defaults. You can change any of those paths later at run-time, the
compiled-in values are merely defaults than can be overridden.</para>

  <para>The <command>configure</command> script is a standard <ulink
url="http://www.gnu.org/software/autoconf/">GNU Autoconf</ulink> script, which
supports all the usual options. If you are not familiar with these scripts,
please refer to the "Running <filename>configure</filename> Scripts" section of
the <application>Autoconf</application> user manual, which is available
at:</para>

  <informalfigure>
    <ulink url="http://www.gnu.org/manual/autoconf/html_chapter/autoconf_13.html#SEC129">http://www.gnu.org/manual/autoconf/html_chapter/autoconf_13.html#SEC129</ulink>
  </informalfigure>

  <para>The only noteworthy addition to the command line is the
<option>--with-debug</option> option, which will tell the build process to
include some debug messages into the binary. You can activate debugging via the
command line, if these messages are compiled in -- what currently is the
default.</para>

  <para>Furthermore, you should expect &httpd; to require a fairly recent C++
compiler to build, because it make full use of the ISO C++ language features. If
you're using the <ulink url="http://gcc.gnu.org/">GNU C Compiler</ulink> version
2.95 or later, you won't have any problems. But other compilers are known not to
be ISO C++ compatible. If you're having trouble, send me an e-mail and I'll see
what I can do.</para>

  <para>If your using a rather current gcc version to compile this package, you
might want to try</para>

  <informalexample>
    <screen>CXXFLAGS=-frepo ./configure</screen>
  </informalexample>

  <para>to configure the build. This will result in a about 30% smaller binary
because unused template instances are optimized out.</para>

  <para>On some platforms, largefile support doesn't work yet. If you're getting
compiler errors, give the configure script the option
<userinput>--disable-largefile</userinput> and try again.</para>

<!--
Local Variables:
mode: sgml
fill-column:80
sgml-parent-document:("httpd.sgml" "article" "sect1")
End:
-->
