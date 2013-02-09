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

#ifndef RESETABLE_VARIABLE_HH_INCLUDED
#define RESETABLE_VARIABLE_HH_INCLUDED

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

#endif // RESETABLE_VARIABLE_HH_INCLUDED
