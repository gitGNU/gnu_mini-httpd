  <para>Compiling the software should be pretty straight forward; all you'll
have to do is the usual routine:</para>

  <screen>
    ./configure
    make
    make install</screen>

  <para>The last step may require super-user privileges, so have the <systemitem
class="username">root</systemitem> password ready if you're going for a
system-wide installation.</para>

  <para>Be adviced that &httpd; needs a fairly recent C++ compiler because it
make full use of the new ISO C++ language features. If you're using the <ulink
url="http://gcc.gnu.org/">GNU C Compiler</ulink> version 2.95 or later, you
won't have any problems. But other compilers are known not to be ISO C++
compatible. If you're having trouble, send me an e-mail and I'll see what I can
do.</para>

  <para>The <filename>configure</filename> script is a standard <ulink
url="http://www.gnu.org/software/autoconf/">GNU Autoconf</ulink> script, which
supports all the usual options. If you are not familiar with these scripts,
please refer to the "Running <filename>configure</filename> Scripts" section of
the <application>Autoconf</application> user manual, which is available
at: </para>

  <informalfigure>
    <ulink url="http://www.gnu.org/manual/autoconf/html_chapter/autoconf_13.html#SEC129">http://www.gnu.org/manual/autoconf/html_chapter/autoconf_13.html#SEC129</ulink>
  </informalfigure>

<!-- Insert lots of text here. -->

  <para>If your using a fairly recent gcc version to compile this, you might
want to try</para>

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
