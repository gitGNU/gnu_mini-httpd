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
    std::string                      host;
    resetable_variable<unsigned int> port;
    std::string                      path;
    std::string                      query;
    };


// This class contains all relevant information in an HTTP request.

struct HTTPRequest
    {
    time_t                           start_up_time;
    std::string                      method;
    URL                              url;
    unsigned int                     major_version;
    unsigned int                     minor_version;
    std::string                      host;
    resetable_variable<unsigned int> port;
    std::string                      connection;
    std::string                      keep_alive;
    resetable_variable<time_t>       if_modified_since;
    std::string                      user_agent;
    std::string                      referer;
    resetable_variable<unsigned int> status_code;
    resetable_variable<size_t>       object_size;
    };

#endif // !defined(HTTPREQUEST_HH)
