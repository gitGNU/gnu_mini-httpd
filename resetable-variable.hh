/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#ifndef RESETABLE_VARIABLE_HH
#define RESETABLE_VARIABLE_HH

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

#endif // !defined(RESETABLE_VARIABLE_HH)
