  <variablelist>

    <varlistentry>
      <term><option>-h</option></term>
      <listitem>
        <para>Show &httpd; usage information.</para>
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
      <term><option>-c <replaceable>config</replaceable></option></term>
      <term><option>--config-file <replaceable>config</replaceable></option></term>
      <listitem>
        <para>Use the configuration file
<filename><replaceable>config</replaceable></filename> rather the
default.</para>
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
