  <variablelist>
    <varlistentry>
      <term><option>-h</option></term>
      <listitem>
        <para>Show &httpd; usage information and exit.</para>
      </listitem>
    </varlistentry>

    <varlistentry>
      <term><option>-d</option></term>
      <term><option>--debug</option></term>
      <listitem>
        <para>Enable debugging. Please note that debugging is only available if
&httpd; has been compiled with the define <symbol>DEBUG</symbol>. Otherwise,
the debug code is not included in the binary.</para>
      </listitem>
    </varlistentry>

    <varlistentry>
      <term><option>--version</option></term>
      <listitem>
        <para>Show httpd's version string and exit.
      </listitem>
    </varlistentry>

    <varlistentry>
      <term><option>-p <replaceable>number</replaceable></option></term>
      <term><option>--port <replaceable>number</replaceable></option></term>
      <listitem>
        <para>Tell &httpd; on which port to listen for incoming requests. The
default port number is <quote>80</quote>.</para>
      </listitem>
    </varlistentry>

    <varlistentry>
      <term><option>-r <replaceable>path</replaceable></option></term>
      <term><option>--change-root <replaceable>path</replaceable></option></term>
      <listitem>
        <para>When started with super-user priviledges, &httpd; will
<citerefentry> <refentrytitle>chroot</refentrytitle> <manvolnum>2</manvolnum>
</citerefentry> to the directory provided with this option. Please note that
once the change of the root directory has taken place, all other paths are
obviously relative to the <emphasis>new</emphasis> root directory! If you
specify <option>-r /usr/local/httpd</option>, for example, the document root
<filename>/usr/local/httpd/htdocs</filename> will be accessed as
<filename>/htdocs</filename>. Keep that in mind when specifying the other
paths!</para>

        <para>The default <function>chroot</function>-directory is the
<varname>prefix</varname> provided to <command>configure</command> when &httpd;
was built. Unless you changed that via the <option>--prefix</option> parameter,
this will be <filename>/usr/local/httpd</filename>.</para>

        <para>If you want to disable the <function>chroot()</function>
altogether, provide the empty string: <option>-r ""</option>. This is not
recommended, though.</para>
      </listitem>
    </varlistentry>

    <varlistentry>
      <term><option>--document-root <replaceable>path</replaceable></option></term>
      <listitem>
        <para>This option sets the document root directory. When an access to,
say, <literal>http://example.org/index.html</literal> comes in, &httpd; will try
to access the file <filename>example.org/index.html</filename> in this
directory. Note that hostnames must always be lower-case in the file system!
Please note also that the path specified here will be interpreted relatively to
the directory that &httpd; <function>chroot()</function>s to.</para>

        <para>The default location is <filename>/htdocs</filename>.</para>
      </listitem>
    </varlistentry>

    <varlistentry>
      <term><option>-l <replaceable>path</replaceable></option></term>
      <term><option>--logfile-directory <replaceable>path</replaceable></option></term>
      <listitem>
        <para>This option sets the directory used by &httpd; to create the
access logfiles. In this directory, one file per hostname will be created.
Please note that the path specified here will be interpreted relatively to the
directory that &httpd; <function>chroot()</function>s to.</para>

        <para>The default location is <filename>/logs</filename>.</para>
      </listitem>
    </varlistentry>

    <varlistentry>
      <term><option>-s <replaceable>string</replaceable></option></term>
      <term><option>--server-string <replaceable>string</replaceable></option></term>
      <listitem>
        <para>This option sets the version string that &httpd; will return with
the <literal>Server:</literal> header in HTTP replies. The default string is
<quote>peti-httpd</quote> -- no actual version number is revealed. If you want
to disable the <literal>Server:</literal> header altogether, specify the empty
string: <option>-s ""</option>.</para>
      </listitem>
    </varlistentry>

    <varlistentry>
      <term><option>-u <replaceable>uid</replaceable></option></term>
      <term><option>--uid <replaceable>uid</replaceable></option></term>
      <listitem>
        <para>This option sets the <emphasis>numeric</emphasis> user id that
&httpd; will <function>setuid()</function> when called with super-user
priviledges. This is necessary because &httpd; must be started as
<literal>root</literal> in order to be able to <function>bind()</function> to
port 80. But you do not want it to run as <literal>root</literal> from then on.
A good choice is usually user id <quote>2</quote> (<literal>daemon</literal>).
If this option is unset, &httpd; will continue to run under the user id is has
been started with.</para>
      </listitem>
    </varlistentry>

    <varlistentry>
      <term><option>-g <replaceable>uid</replaceable></option></term>
      <term><option>--gid <replaceable>uid</replaceable></option></term>
      <listitem>
        <para>This option sets the <emphasis>numeric</emphasis> group id that
&httpd; will <function>setuid()</function> when called with super-use
priviledges. (See <option>--uid</option> for further details.)</para>
      </listitem>
    </varlistentry>

    <varlistentry>
      <term><option>--default-page <replaceable>filename</replaceable></option></term>
      <listitem>
        <para>This option sets the filename that &httpd; will try to access when
a request for a directory is received. The default is
<filename>index.html</filename>.</para>
      </listitem>
    </varlistentry>

    <varlistentry>
      <term><option>-D</option></term>
      <term><option>--no-detach</option></term>
      <listitem>
        <para>This option tells &httpd; not to detach from the controlling
terminal but to run in the foreground. The default is to detach.</para>
      </listitem>
    </varlistentry>

    <varlistentry>
      <term><option>-H <replaceable>string</replaceable></option></term>
      <term><option>--default-hostname <replaceable>string</replaceable></option></term>
      <listitem>
        <para>This option sets default host name &httpd; should use when a pre
HTTP/1.1 request is received that does not specify the <literal>Host:</literal>
header. If the default hostname is empty or this option is omitted, &httpd; will
reject requests that do not provide the <literal>Host:</literal> header or give
the complete URL in the request.</para>
      </listitem>
    </varlistentry>

  </variablelist>

<!--
Local Variables:
mode: sgml
fill-column:80
sgml-parent-document:("httpd.sgml" "article" "sect1" "title" "para")
End:
-->
