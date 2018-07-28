/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_COLLECTIONS_VECTOR_HXX

#include <lofty/collections/vector-0.hxx>

#ifndef _LOFTY_NOPUB
   #define _LOFTY_NOPUB
   #define _LOFTY_COLLECTIONS_VECTOR_HXX
#endif

#ifndef _LOFTY_COLLECTIONS_VECTOR_HXX_NOPUB
#define _LOFTY_COLLECTIONS_VECTOR_HXX_NOPUB

#include <lofty/from_text_istream.hxx>
#include <lofty/_std/functional.hxx>
#include <lofty/to_text_ostream.hxx>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace collections { namespace _pvt {

/*! Base class for the specializations of from_text_istream for vector types. Not using templates, so the
implementation can be in a cxx file. */
class LOFTY_SYM vector_from_text_istream : public lofty::_pvt::sequence_from_text_istream {
public:
   //! Default constructor.
   vector_from_text_istream();

   //! Destructor.
   ~vector_from_text_istream();
};

}}} //namespace lofty::collections::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! @cond
namespace lofty {

template <typename T, std::size_t embedded_capacity>
class from_text_istream<collections::_LOFTY_PUBNS vector<T, embedded_capacity>> :
   public collections::_pvt::vector_from_text_istream {
private:
   typedef collections::_pvt::vector_from_text_istream vector_from_text_istream;

public:
   /*! Converts a capture into a value of the appropriate type.

   @param capture0
      Pointer to the top-level capture.
   @param dst
      Pointer to the destination object.
   */
   void convert_capture(
      text::parsers::_LOFTY_PUBNS dynamic_match_capture const & capture0,
      collections::_LOFTY_PUBNS vector<T, embedded_capacity> * dst
   ) {
      std::size_t count = vector_from_text_istream::captures_count(capture0);
      dst->set_size(count);
      for (std::size_t i = 0; i < count; ++i) {
         auto const & elt_capture = vector_from_text_istream::capture_at(capture0, i);
         elt_ftis.convert_capture(elt_capture, &(*dst)[static_cast<std::ptrdiff_t>(i)]);
      }
   }

   /*! Creates parser states for the specified input format.

   @param format
      Formatting options.
   @param parser
      Pointer to the parser instance to use to create non-static states.
   @return
      First parser state.
   */
   text::parsers::_LOFTY_PUBNS dynamic_state const * format_to_parser_states(
      text::parsers::_LOFTY_PUBNS regex_capture_format const & format,
      text::parsers::_LOFTY_PUBNS dynamic * parser
   ) {
      auto const & elt_format = vector_from_text_istream::extract_elt_format(format);
      auto elt_first_state = elt_ftis.format_to_parser_states(elt_format, parser);
      return vector_from_text_istream::format_to_parser_states(format, parser, elt_first_state);
   }

protected:
   //! Backend for the individual elements.
   from_text_istream<T> elt_ftis;
};

} //namespace lofty
//! @endcond

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace collections { namespace _pvt {

/*! Base class for the specializations of to_text_ostream for vector types. Not using templates, so the
implementation can be in a cxx file. */
class LOFTY_SYM vector_to_text_ostream : public lofty::_pvt::sequence_to_text_ostream {
public:
   //! Default constructor.
   vector_to_text_ostream();

   //! Destructor.
   ~vector_to_text_ostream();

   /*! Changes the output format.

   @param format
      Formatting options.
   @return
      Formatting options to be applied to each individual element.
   */
   text::_LOFTY_PUBNS str set_format(text::_LOFTY_PUBNS str const & format);
};

}}} //namespace lofty::collections::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! @cond
namespace lofty {

template <typename T, std::size_t embedded_capacity>
class to_text_ostream<collections::_LOFTY_PUBNS vector<T, embedded_capacity>> :
   public collections::_pvt::vector_to_text_ostream {
public:
   /*! Changes the output format.

   @param format
      Formatting options.
   */
   void set_format(text::_LOFTY_PUBNS str const & format) {
      auto elt_format(collections::_pvt::vector_to_text_ostream::set_format(format));
      elt_ttos.set_format(elt_format);
   }

   /*! Writes a vector, applying the formatting options.

   @param src
      Object to write.
   @param dst
      Pointer to the stream to output to.
   */
   void write(
      collections::_LOFTY_PUBNS vector<T, embedded_capacity> const & src, io::text::_LOFTY_PUBNS ostream * dst
   ) {
      _write_start(dst);
      auto itr(src.cbegin()), end(src.cend());
      if (itr != end) {
         elt_ttos.write(*itr, dst);
         while (++itr != end) {
            _write_separator(dst);
            elt_ttos.write(*itr, dst);
         }
      }
      _write_end(dst);
   }

protected:
   //! Backend for the individual elements.
   to_text_ostream<T> elt_ttos;
};

template <typename T>
class to_text_ostream<collections::_LOFTY_PUBNS vector_const_iterator<T>> :
   public to_text_ostream<typename collections::_LOFTY_PUBNS vector_const_iterator<T>::pointer> {
public:
   /*! Writes an iterator as a pointer, applying the formatting options.

   @param src
      Object to write.
   @param dst
      Pointer to the stream to output to.
   */
   void write(
      collections::_LOFTY_PUBNS vector_const_iterator<T> const & src, io::text::_LOFTY_PUBNS ostream * dst
   ) {
      to_text_ostream<typename collections::_pub::vector_const_iterator<T>::pointer>::write(&*src, dst);
   }
};

template <typename T>
class to_text_ostream<collections::_LOFTY_PUBNS vector_iterator<T>> :
   public to_text_ostream<collections::_LOFTY_PUBNS vector_const_iterator<T>> {
};

} //namespace lofty
//! @endcond

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! @cond
namespace std {

template <typename T, std::size_t embedded_capacity>
struct LOFTY_SYM hash<lofty::collections::_LOFTY_PUBNS vector<T, embedded_capacity>> {
public:
   std::size_t operator()(lofty::collections::_LOFTY_PUBNS vector<T, embedded_capacity> const & v) const {
      std::size_t ret = v.size();
      LOFTY_FOR_EACH(auto const & elt, v) {
         ret ^= elt_hash(elt);
      }
      return ret;
   }

private:
   hash<T> elt_hash;
};

} //namespace std
//! @endcond

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_COLLECTIONS_VECTOR_HXX_NOPUB

#ifdef _LOFTY_COLLECTIONS_VECTOR_HXX
   #undef _LOFTY_NOPUB

   #ifdef LOFTY_CXX_PRAGMA_ONCE
      #pragma once
   #endif
#endif

#endif //ifndef _LOFTY_COLLECTIONS_VECTOR_HXX
