/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#ifndef CONFIG_HH
#define CONFIG_HH

#if defined(HAVE_EXT_HASH_MAP)
#  include <ext/hash_map>
#elif defined(HAVE_HASH_MAP)
#  include <hash_map>
#else
#  error Do not know how to include a hash map
#endif
#include <string>
#include <sys/types.h>
#include "resetable-variable.hh"

class configuration
    {
  public:
    // Construction and Destruction.
    explicit configuration(int, char**);
    ~configuration();

    // Timeouts.
    static unsigned int network_read_timeout;
    static unsigned int network_write_timeout;
    static unsigned int hard_poll_interval_threshold;
    static int          hard_poll_interval;

    // Buffer sizes.
    static unsigned int max_line_length;

    // Paths.
    static std::string  chroot_directory;
    static std::string  logfile_directory;
    static std::string  document_root;
    static std::string  default_page;

    // Run-time stuff.
    static char*                      default_content_type;
    static unsigned int               http_port;
    static std::string                server_string;
    static resetable_variable<uid_t>  setuid_user;
    static resetable_variable<gid_t>  setgid_group;
    static bool                       debugging;

    // Content-type mapping.
    const char* get_content_type(const char* filename) const;

    // The error class throw in case version or usage information has
    // been requested and we're supposed to terminate.
    struct no_error { };

  private:
    configuration(const configuration&);
    configuration& operator= (const configuration&);

    struct eqstr
	{
	bool operator()(const char* lhs, const char* rhs) const
	    { return strcasecmp(lhs, rhs) == 0; }
	};
    typedef std::hash_map<const char*,const char*,std::hash<const char*>,eqstr> map_t;
    map_t content_types;
    };
extern const configuration* config;

#endif
