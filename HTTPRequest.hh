/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#ifndef HTTPREQUEST_HH
#define HTTPREQUEST_HH

#include <string>
#include <ostream>
#include <ctime>

// This is a wrapper class about variables where you want to keep
// track of whether it has been assigened yet or not.

template <typename varT>
class resetable_variable
    {
  public:
    typedef varT value_type;

    resetable_variable()
            : is_set(false)
        {
        }
    resetable_variable(const value_type& val)
            : value(val), is_set(true)
        {
        }
    resetable_variable<value_type>& operator= (const value_type& rhs)
        {
        value = rhs;
        is_set = true;
        return *this;
        }
    const value_type& data() const throw()
        {
        return value;
        }
    operator const value_type&() const throw()
        {
        return value;
        }
    bool empty() const throw()
        {
        return !is_set;
        }

  private:
    value_type value;
    bool is_set;
    };

template<typename varT>
inline std::ostream& operator<< (std::ostream& os, const resetable_variable<varT>& var)
    {
    os << static_cast<varT>(var);
    return os;
    }

// This class contains the relevant information in an (HTTP) URL.

struct URL
    {
    resetable_variable<std::string>  host;
    bool                             host_is_ip_address;
    resetable_variable<unsigned int> port;
    resetable_variable<std::string>  path;
    resetable_variable<std::string>  query;
    };


// This class contains all relevant information in an HTTP request.

struct HTTPRequest
    {
    std::string                      method;
    URL                              url;
    unsigned int                     major_version;
    unsigned int                     minor_version;
    resetable_variable<std::string>  host;
    bool                             host_is_ip_address;
    resetable_variable<unsigned int> port;
    resetable_variable<std::string>  connection;
    resetable_variable<std::string>  keep_alive;
    resetable_variable<time_t>       if_not_modified_since;
    resetable_variable<std::string>  user_agent;
    resetable_variable<std::string>  referer;
    };

#endif // !defined(HTTPREQUEST_HH)
