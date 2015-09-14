/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014, 2015
Raffaello D. Di Napoli

This file is part of Abaclade.

Abaclade is free software: you can redistribute it and/or modify it under the terms of the GNU
General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Abaclade is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
Public License for more details.

You should have received a copy of the GNU General Public License along with Abaclade. If not, see
<http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#ifndef _ABACLADE_HXX_INTERNAL
   #error "Please #include <abaclade.hxx> instead of this file"
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

/*! Makes a derived class not copyable.

Derive a class from this for maximum compatibility instead of explicitly deleting copy constructor
and copy assignment operator.

This utility class is affected by a known MSC18 (Visual Studio 2013) bug, due to which for that
compiler Abaclade will override std::is_copy_constructible with a fixed version.

With MSC18, Microsoft introduced a completely broken std::is_copy_constructible that makes one wish
they didn’t: it returns true for classes with private copy constructors[1], classes with deleted
constructors[2], and classes composed by non-copyable classes[2].

The only way of fixing it without breaking MSC’s STL implementation is to override and enhance it
with Abaclade’s version, making the latter fall back to MSC’s broken implementation. In the worst
case, this might make it report classes as non-copyable when they are, but it will (hopefully) not
report non-copyable classes as copyable.

[1] <https://connect.microsoft.com/VisualStudio/feedback/details/799732/is-copy-constructible-
   always-returns-true-for-private-copy-constructors>
[2] <https://connect.microsoft.com/VisualStudio/feedback/details/800328/std-is-copy-
   constructible-is-broken>
*/
class noncopyable {
protected:
   //! Default constructor. Protected to prevent instantiations of this class as-is.
   noncopyable() {
   }

   //! Destructor. Protected to deter using this class as a polymorphic base.
   ~noncopyable() {
   }

#ifdef ABC_CXX_FUNC_DELETE
   noncopyable(noncopyable const &) = delete;
   noncopyable & operator=(noncopyable const &) = delete;
#else
private:
   noncopyable(noncopyable const &);
   noncopyable & operator=(noncopyable const &);
#endif
};

} //namespace abc
