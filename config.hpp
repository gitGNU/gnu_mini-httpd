/*
 * Copyright (c) 2001-2007 Peter Simons <simons@cryp.to>
 *
 * This software is provided 'as-is', without any express or
 * implied warranty. In no event will the authors be held liable
 * for any damages arising from the use of this software.
 *
 * Copying and distribution of this file, with or without
 * modification, are permitted in any medium without royalty
 * provided the copyright notice and this notice are preserved.
 */

#ifndef CONFIG_HH
#define CONFIG_HH

#include <map>
#include <string>
#include <sys/types.h>
#include <boost/optional.hpp>
#include <boost/noncopyable.hpp>

class configuration : private boost::noncopyable
{
public:
  // Construction and Destruction.
  configuration(int, char**);

  // Timeouts.
  unsigned int network_read_timeout;
  unsigned int network_write_timeout;
  unsigned int hard_poll_interval_threshold;
  int          hard_poll_interval;

  // Buffer sizes.
  unsigned int max_line_length;

  // Paths.
  std::string  chroot_directory;
  std::string  logfile_directory;
  std::string  document_root;
  std::string  default_page;

  // Run-time stuff.
  char const *               default_content_type;
  std::string                default_hostname;
  unsigned int               http_port;
  std::string                server_string;
  boost::optional<uid_t>     setuid_user;
  boost::optional<gid_t>     setgid_group;
  bool                       debugging;
  bool                       detach;

  // Content-type mapping.
  char const * get_content_type(char const * filename) const;

  // The error class throw in case version or usage information has
  // been requested and we're supposed to terminate.
  struct no_error { };

private:
  struct ltstr
  {
    bool operator()(char const * lhs, char const * rhs) const
    {
      return ::strcasecmp(lhs, rhs) < 0;
    }
  };

  typedef std::map<char const *, char const *, ltstr> map_t;
  map_t content_types;
};

extern configuration const * config;

#endif
