  <para>The good news is that getting &httpd; up and running is trivial, because
the program does have the least possible set of features. It may look weird at
first, especially if you know other web servers like
<application>Apache</application>, but I guess you'll like it.</para>

  <para>In this section, I assume that you have successfully built and installed
the binary itself. I furthermore assume that you stuck to the default
installation prefix of <filename>/usr/local/httpd</filename>. If you did not,
then apparently you know what you're doing and will probably be able to adapt to
the changes easily without having me pointing everything out all the
time.</para>

  <para>The first thing to know is that &httpd; will call <citerefentry>
      <refentrytitle>chroot</refentrytitle>
      <manvolnum>2</manvolnum>
    </citerefentry> to <filename>/usr/local/httpd</filename> as soon as it has
initialized its internals. This means, that all files related to the web service
must reside within this directory. Please note that you cannot symlink to some
path outside that hierarchy or anything, the rest of your machine will not be
accessible for &httpd;!</para>

  <para>The first thing you need to decide is under which user id you'll run
&httpd;. A sensible choice is <literal>daemon</literal>, but you can as well use
<literal>nobody</literal> or any other user you see fit. Find out which numeric
user id this user has on your system by calling the <citerefentry>
      <refentrytitle>id</refentrytitle>
      <manvolnum>1</manvolnum>
    </citerefentry> system tool:</para>

  <informalexample>
    <screen><computeroutput>$</computeroutput> <userinput>id daemon</userinput>
<computeroutput>uid=2(daemon) gid=2(daemon) groups=2(daemon),1(bin)</computeroutput></screen>
  </informalexample>

  <para>Now you need to create two directories, which must be accessible for
this user: <filename>htdocs</filename> and <filename>logs</filename>. The first
directory will contain the actual web documents in a subdirectory per (virtual)
host you want to provide and the second directory will contain the access logs
written by &httpd;. Consequently, &httpd; must have read permissions on
<filename>htdocs</filename> and write permission on <filename>logs</filename>.
The following commands will ensure that:</para>

  <informalexample>
    <screen># mkdir htdocs logs
# chown daemon.daemon htdocs logs</screen>
  </informalexample>

  <para>Now create a directory in <filename>htdocs</filename>, which is called
like the hostname under which you want to provide the web service. Make sure you
use all lower-case characters! If your machine may be accessed as, say,
<literal>example.com</literal> and <literal>www.example.com</literal>, do the
following:</para>

  <informalexample>
    <screen># mkdir htdocs/example.org
# ln -s htdocs/example.org htdocs/www.example.org
# chown daemon.daemon htdocs/example.org htdocs/www.example.org</screen>
  </informalexample>

  <para>If you're unsure what hostname to use here for testing purposes,
<literal>localhost</literal> is usually a safe bet. :-)</para>

  <para>Copy the HTML documents to the <filename>htdocs/example.org</filename>
directory and again make sure that &httpd; has permission to access them!
Finally, start &httpd; as <literal>root</literal> with the following command
line:</para>

  <informalexample>
    <screen># /usr/local/httpd/bin/httpd --uid 2</screen>
  </informalexample>

  <para>Now you can access the URL <literal>http://example.org/</literal> with
you favorite broweser. That's it.</para>

  <para>In case you want to try &httpd; with some port other than 80, add the
<option>--port</option> option to the command line: <option>--port
8080</option>. Furthermore, you might want to add the <option>--debug</option>
switch to turn on the debugging messages in &httpd;. That is particularly
helpful in case &httpd; is not behaving as you'd expect.</para>

  <para>Once you have accessed the web site, you'll find the apropriate access
logfile in the <filename>logs</filename> directory, one per virtual host.</para>

  <para>If you want to add more virtual hosts to &httpd;, just create the
apropriate directory in <filename>htdocs</filename>.</para>

<!--
Local Variables:
mode: sgml
fill-column:80
sgml-parent-document:("httpd.sgml" "article" "sect1")
End:
-->
