/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include "config.hh"
#include "log.hh"
using namespace std;

#define kb * 1024
#define sec * 1

// Timeouts.
unsigned int configuration::network_read_timeout        = 30 sec;
unsigned int configuration::network_write_timeout       = 30 sec;
unsigned int configuration::file_read_timeout           =  0 sec;

// Buffer sizes.
unsigned int configuration::io_buffer_size              =  4 kb;

// Paths.
std::string configuration::document_root                = DOCUMENT_ROOT;

// Miscellaneous.
char* configuration::default_content_type		= "application/octet-stream";

configuration::configuration()
    {
    TRACE();
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
    }

configuration::~configuration()
    {
    TRACE();
    }

const char* configuration::get_content_type(const string& filename) const
    {
    TRACE();
    string::size_type pos = filename.rfind('.');
    if (pos == string::npos || pos+1 == filename.size())
	{
	debug("get_content_type(): Can't find suffix in filename '%s'; using default.", filename.c_str());
	return default_content_type;
	}
    string suffix = filename.substr(pos+1, string::npos);
    debug("get_content_type(): Filename '%s' has suffix '%s'.", filename.c_str(), suffix.c_str());
    map<string,string>::const_iterator i = content_types.find(suffix);
    if (i != content_types.end())
	{
	debug("get_content_type(): suffix '%s' maps to content type '%s'.", suffix.c_str(), i->second.c_str());
 	return i->second.c_str();
	}
    else
	{
	debug("get_content_type(): No content type found for suffix '%s'; using default.", suffix.c_str());
	return default_content_type;
	}
    }
