  <para>&httpd; is a minimalistic web server designed for optimal performance,
high security, and as little use of system resources as possible. Unlike other
popular web servers, &httpd; does not require multiple processes to handle
multiple requests in parallel. By using asynchronous I/O and multiplexing all
open connections internally, &httpd is thus able to maintain a virtually
arbitrary number of concurrent HTTP requests.</para>

<!--
Local Variables:
mode: sgml
fill-column:80
sgml-parent-document:("httpd.sgml" "article" "sect1")
End:
-->
