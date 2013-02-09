/*
 * Copyright (c) 2001-2013 Peter Simons <simons@cryp.to>
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <getopt.h>
#include "log.hh"
#include "config.hh"
#include "config.h"
using namespace std;

#define kb * 1024
#define mb * 1024 kb
#define sec * 1

// Timeouts.
unsigned int configuration::network_read_timeout         = 30 sec;
unsigned int configuration::network_write_timeout        = 30 sec;

unsigned int configuration::hard_poll_interval_threshold = 32;
int configuration::hard_poll_interval                    = 60 sec;

// Buffer sizes.
unsigned int configuration::max_line_length              =  4 kb;

// Paths.
string configuration::chroot_directory                   = PREFIX;
string configuration::logfile_directory                  = "/logs";
string configuration::document_root                      = "/htdocs";
string configuration::default_page                       = "index.html";

// Run-time stuff.
string configuration::server_string                      = PACKAGE_NAME;
string configuration::default_hostname;
char const * configuration::default_content_type         = "application/octet-stream";
long int configuration::http_port                        = 80;
resetable_variable<uid_t> configuration::setuid_user;
resetable_variable<gid_t> configuration::setgid_group;
bool configuration::debugging                            = false;
bool configuration::detach                               = true;

#define USAGE_MSG \
  "Usage: httpd [-h | --help] [--version] [-d | --debug]\n" \
  "    [-p number | --port number] [-r path | --change-root path]\n" \
  "    [--document-root path] [-l path | --logfile-directory path]\n" \
  "    [-s string | --server-string string] [-u uid | --uid uid]\n" \
  "    [-g gid | --gid gid] [--default-page filename]\n"

configuration::configuration(int argc, char** argv)
{
  TRACE();

  // Parse the command line.

  const char* optstring = "hdp:r:l:s:u:g:DH:";
  const option longopts[] =
  {
    { "help",               no_argument,       0, 'h' },
    { "version",            no_argument,       0, 'v' },
    { "debug",              no_argument,       0, 'd' },
    { "port",               required_argument, 0, 'p' },
    { "change-root",        required_argument, 0, 'r' },
    { "logfile-directory",  required_argument, 0, 'l' },
    { "server-string",      required_argument, 0, 's' },
    { "uid",                required_argument, 0, 'u' },
    { "gid",                required_argument, 0, 'g' },
    { "no-detach",          required_argument, 0, 'D' },
    { "default-hostname",   required_argument, 0, 'H' },
    { "document-root",      required_argument, 0, 'y' },
    { "default-page",       required_argument, 0, 'z' },
    { 0, 0, 0, 0 }          // mark end of array
  };
  int rc;
  opterr = 0;
  while ((rc = getopt_long(argc, argv, optstring, longopts, 0)) != -1)
  {
    switch (rc)
    {
      case 'h':
        fprintf(stderr, USAGE_MSG);
        throw no_error();
      case 'v':
        printf("%s version %s\n", PACKAGE_NAME, PACKAGE_VERSION);
        throw no_error();
      case 'd':
        debugging = true;
        break;
      case 'p':
        http_port = strtol(optarg, 0, 10);
        if (http_port <= 0 || http_port > 65535)
          throw runtime_error("The specified port number is out of range!");
        break;
      case 'g':
        setgid_group = strtol(optarg, 0, 10);
        break;
      case 'u':
        setuid_user = strtol(optarg, 0, 10);
        break;
      case 'r':
        chroot_directory = optarg;
        break;
      case 'y':
        document_root = optarg;
        break;
      case 'z':
        default_page = optarg;
        break;
      case 'l':
        logfile_directory = optarg;
        break;
      case 's':
        server_string = optarg;
        break;
      case 'D':
        detach = false;
        break;
      case 'H':
        default_hostname = optarg;
        break;
      default:
        fprintf(stderr, USAGE_MSG);
        throw runtime_error("Incorrect command line syntax.");
    }

    // Consistency checks on the configured parameters.

    if (default_page.empty())
      throw invalid_argument("Setting an empty --default-page is not allowed.");
    if (document_root.empty())
      throw invalid_argument("Setting an empty --document-root is not allowed.");
    if (logfile_directory.empty())
      throw invalid_argument("Setting an empty --document-root is not allowed.");
  }

  // Initialize the content type lookup map.

  content_types["ai"]      = "application/postscript";
  content_types["aif"]     = "audio/x-aiff";
  content_types["aifc"]    = "audio/x-aiff";
  content_types["aiff"]    = "audio/x-aiff";
  content_types["asc"]     = "text/plain";
  content_types["au"]      = "audio/basic";
  content_types["avi"]     = "video/x-msvideo";
  content_types["bcpio"]   = "application/x-bcpio";
  content_types["bmp"]     = "image/bmp";
  content_types["cdf"]     = "application/x-netcdf";
  content_types["cpio"]    = "application/x-cpio";
  content_types["cpt"]     = "application/mac-compactpro";
  content_types["csh"]     = "application/x-csh";
  content_types["css"]     = "text/css";
  content_types["dcr"]     = "application/x-director";
  content_types["dir"]     = "application/x-director";
  content_types["doc"]     = "application/msword";
  content_types["dvi"]     = "application/x-dvi";
  content_types["dxr"]     = "application/x-director";
  content_types["eps"]     = "application/postscript";
  content_types["etx"]     = "text/x-setext";
  content_types["gif"]     = "image/gif";
  content_types["gtar"]    = "application/x-gtar";
  content_types["hdf"]     = "application/x-hdf";
  content_types["hqx"]     = "application/mac-binhex40";
  content_types["htm"]     = "text/html";
  content_types["html"]    = "text/html";
  content_types["ice"]     = "x-conference/x-cooltalk";
  content_types["ief"]     = "image/ief";
  content_types["iges"]    = "model/iges";
  content_types["igs"]     = "model/iges";
  content_types["jpe"]     = "image/jpeg";
  content_types["jpeg"]    = "image/jpeg";
  content_types["jpg"]     = "image/jpeg";
  content_types["js"]      = "application/x-javascript";
  content_types["kar"]     = "audio/midi";
  content_types["latex"]   = "application/x-latex";
  content_types["man"]     = "application/x-troff-man";
  content_types["me"]      = "application/x-troff-me";
  content_types["mesh"]    = "model/mesh";
  content_types["mid"]     = "audio/midi";
  content_types["midi"]    = "audio/midi";
  content_types["mov"]     = "video/quicktime";
  content_types["movie"]   = "video/x-sgi-movie";
  content_types["mp2"]     = "audio/mpeg";
  content_types["mp3"]     = "audio/mpeg";
  content_types["mpe"]     = "video/mpeg";
  content_types["mpeg"]    = "video/mpeg";
  content_types["mpg"]     = "video/mpeg";
  content_types["mpga"]    = "audio/mpeg";
  content_types["ms"]      = "application/x-troff-ms";
  content_types["msh"]     = "model/mesh";
  content_types["nc"]      = "application/x-netcdf";
  content_types["oda"]     = "application/oda";
  content_types["pbm"]     = "image/x-portable-bitmap";
  content_types["pdb"]     = "chemical/x-pdb";
  content_types["pdf"]     = "application/pdf";
  content_types["pgm"]     = "image/x-portable-graymap";
  content_types["pgn"]     = "application/x-chess-pgn";
  content_types["png"]     = "image/png";
  content_types["pnm"]     = "image/x-portable-anymap";
  content_types["ppm"]     = "image/x-portable-pixmap";
  content_types["ppt"]     = "application/vnd.ms-powerpoint";
  content_types["ps"]      = "application/postscript";
  content_types["qt"]      = "video/quicktime";
  content_types["ra"]      = "audio/x-realaudio";
  content_types["ram"]     = "audio/x-pn-realaudio";
  content_types["ras"]     = "image/x-cmu-raster";
  content_types["rgb"]     = "image/x-rgb";
  content_types["rm"]      = "audio/x-pn-realaudio";
  content_types["roff"]    = "application/x-troff";
  content_types["rpm"]     = "audio/x-pn-realaudio-plugin";
  content_types["rtf"]     = "application/rtf";
  content_types["rtf"]     = "text/rtf";
  content_types["rtx"]     = "text/richtext";
  content_types["sgm"]     = "text/sgml";
  content_types["sgml"]    = "text/sgml";
  content_types["sh"]      = "application/x-sh";
  content_types["shar"]    = "application/x-shar";
  content_types["silo"]    = "model/mesh";
  content_types["sit"]     = "application/x-stuffit";
  content_types["skd"]     = "application/x-koan";
  content_types["skm"]     = "application/x-koan";
  content_types["skp"]     = "application/x-koan";
  content_types["skt"]     = "application/x-koan";
  content_types["snd"]     = "audio/basic";
  content_types["spl"]     = "application/x-futuresplash";
  content_types["src"]     = "application/x-wais-source";
  content_types["sv4cpio"] = "application/x-sv4cpio";
  content_types["sv4crc"]  = "application/x-sv4crc";
  content_types["swf"]     = "application/x-shockwave-flash";
  content_types["t"]       = "application/x-troff";
  content_types["tar"]     = "application/x-tar";
  content_types["tcl"]     = "application/x-tcl";
  content_types["tex"]     = "application/x-tex";
  content_types["texi"]    = "application/x-texinfo";
  content_types["texinfo"] = "application/x-texinfo";
  content_types["tif"]     = "image/tiff";
  content_types["tiff"]    = "image/tiff";
  content_types["tr"]      = "application/x-troff";
  content_types["tsv"]     = "text/tab-separated-values";
  content_types["txt"]     = "text/plain";
  content_types["ustar"]   = "application/x-ustar";
  content_types["vcd"]     = "application/x-cdlink";
  content_types["vrml"]    = "model/vrml";
  content_types["wav"]     = "audio/x-wav";
  content_types["wrl"]     = "model/vrml";
  content_types["xbm"]     = "image/x-xbitmap";
  content_types["xls"]     = "application/vnd.ms-excel";
  content_types["xml"]     = "text/xml";
  content_types["xpm"]     = "image/x-xpixmap";
  content_types["xwd"]     = "image/x-xwindowdump";
  content_types["xyz"]     = "chemical/x-pdb";
  content_types["zip"]     = "application/zip";
  content_types["hpp"]     = "text/plain";
  content_types["cpp"]     = "text/plain";
}

configuration::~configuration()
{
  TRACE();
}

const char* configuration::get_content_type(const char* filename) const
{
  const char* last_dot;
  const char* current;
  for (current = filename, last_dot = 0; *current != '\0'; ++current)
    if (*current == '.')
      last_dot = current;
  if (last_dot == 0)
    return default_content_type;
  else
    ++last_dot;
  map_t::const_iterator i = content_types.find(last_dot);
  if (i != content_types.end())
    return i->second;
  else
    return default_content_type;
}
