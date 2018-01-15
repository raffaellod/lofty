/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>
#include <lofty/bitmanip.hxx>
#include <lofty/byte_order.hxx>
#include <lofty/destructing_unfinalized_object.hxx>
#include <lofty/event.hxx>
#include <lofty/from_str.hxx>
#include <lofty/io/text.hxx>
#include <lofty/logging.hxx>
#include <lofty/math.hxx>
#include <lofty/text.hxx>
#include <lofty/text/parsers/dynamic.hxx>
#include <lofty/text/parsers/regex.hxx>
#include <lofty/thread.hxx>
#include <lofty/type_void_adapter.hxx>
#include "coroutine-scheduler.hxx"

#include <cstdlib> // std::abort() std::free() std::malloc() std::realloc()
#if LOFTY_HOST_API_POSIX
   #include <unistd.h> // _SC_* sysconf()
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if LOFTY_HOST_API_WIN32
/*! Entry point for lofty.dll.

@param hinst
   Module’s instance handle.
@param reason
   Reason why this function was invoked; one of DLL_{PROCESS,THREAD}_{ATTACH,DETACH}.
@param reserved
   Legacy.
@return
   true in case of success, or false otherwise.
*/
extern "C" ::BOOL WINAPI DllMain(::HINSTANCE hinst, ::DWORD reason, void * reserved) {
   LOFTY_UNUSED_ARG(hinst);
   LOFTY_UNUSED_ARG(reserved);
   if (!lofty::_pvt::thread_local_storage::dllmain_hook(static_cast<unsigned>(reason))) {
      return false;
   }
   return true;
}
#endif //if LOFTY_HOST_API_WIN32

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

destructing_unfinalized_object::destructing_unfinalized_object(destructing_unfinalized_object const & src) :
   exception(src) {
}

/*virtual*/ destructing_unfinalized_object::~destructing_unfinalized_object() LOFTY_STL_NOEXCEPT_TRUE() {
}

destructing_unfinalized_object & destructing_unfinalized_object::operator=(
   destructing_unfinalized_object const & src
) {
   exception::operator=(src);
   return *this;
}

void destructing_unfinalized_object::write_what(void const * o, _std::type_info const & type) {
   what_ostream().print(
      LOFTY_SL("instance of {} @ {} being destructed before finalize() was invoked on it"), type, o
   );
}

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

event::event() :
   coro_sched_w(this_thread::coroutine_scheduler()),
   id(0) {
   if (auto coro_sched = coro_sched_w.lock()) {
      id = coro_sched->create_event();
   } else {
      // TODO: create a non-coroutine event (reuse simple_event?).
   }
}

event::event(event && src) :
   coro_sched_w(_std::move(src.coro_sched_w)),
   id(src.id) {
   src.id = 0;
}

event::~event() {
   if (auto coro_sched = coro_sched_w.lock()) {
      coro_sched->discard_event(id);
   } else {
      // TODO: see constructor.
   }
}

event & event::operator=(event && src) {
   coro_sched_w = _std::move(src.coro_sched_w);
   id = src.id;
   src.id = 0;
   return *this;
}

void event::trigger() {
   if (auto coro_sched = coro_sched_w.lock()) {
      coro_sched->trigger_event(id);
   } else {
      // TODO: see constructor.
   }
}

void event::wait(unsigned timeout_ms /*= 0*/) {
   if (auto coro_sched = coro_sched_w.lock()) {
      coro_sched->block_active(
         timeout_ms, id, io::filedesc_t_null, false /*read – N/A*/
#if LOFTY_HOST_API_WIN32
         , nullptr
#endif
      );
   } else {
      // TODO: see constructor.
   }
}

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

/*static*/ void type_void_adapter::copy_construct_trivial_impl(
   std::int8_t * dst_bytes_begin, std::int8_t * src_bytes_begin, std::int8_t * src_bytes_end
) {
   memory::copy(dst_bytes_begin, src_bytes_begin, static_cast<std::size_t>(src_bytes_end - src_bytes_begin));
}

/*static*/ void type_void_adapter::destruct_trivial_impl(void const * begin, void const * end) {
   LOFTY_UNUSED_ARG(begin);
   LOFTY_UNUSED_ARG(end);
   // Nothing to do.
}

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _pvt {

void context_local_storage_registrar_impl::add_var(
   context_local_storage_node_impl * var, std::size_t var_byte_size
) {
   var->storage_index = vars_count++;
   // Calculate the offset for *var’s storage and increase var_size accordingly.
   var->storage_byte_offset = vars_byte_size;
   vars_byte_size += bitmanip::ceiling_to_pow2_multiple(var_byte_size, sizeof(_std::max_align_t));
   if (frozen_byte_size != 0 && vars_byte_size > frozen_byte_size) {
      // TODO: can’t log/report anything since no thread locals are available! Fix me!
      std::abort();
   }
}

}} //namespace lofty::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _pvt {

context_local_storage_impl::context_local_storage_impl(context_local_storage_registrar_impl * registrar) :
   vars_constructed(new bool[registrar->vars_count]),
   bytes(new std::int8_t[registrar->vars_byte_size]) {
   memory::clear(vars_constructed.get(), registrar->vars_count);
   memory::clear(bytes.get(), registrar->vars_byte_size);
   if (registrar->frozen_byte_size == 0) {
      // Track the size of this first block.
      registrar->frozen_byte_size = registrar->vars_byte_size;
   }
}

context_local_storage_impl::~context_local_storage_impl() {
}

bool context_local_storage_impl::destruct_vars(context_local_storage_registrar_impl const & registrar) {
   bool any_destructed = false;
   // Iterate backwards over the list to destruct TLS/CRLS for this storage.
   unsigned i = registrar.vars_count;
   for (auto itr(registrar.rbegin()), end(registrar.rend()); itr != end; ++itr) {
      auto & var = static_cast<context_local_storage_node_impl &>(*itr);
      if (vars_constructed[--i]) {
         if (var.destruct) {
            var.destruct(&bytes[var.storage_byte_offset]);
            /* Only set any_destructed if we executed a destructor: if we didn’t, it can’t have re-constructed
            any other variables. */
            any_destructed = true;
         }
         vars_constructed[i] = false;
      }
   }
   return any_destructed;
}

void * context_local_storage_impl::get_storage(context_local_storage_node_impl const & var) {
   void * ret = &bytes[var.storage_byte_offset];
   if (!vars_constructed[var.storage_index]) {
      if (var.construct) {
         var.construct(ret);
      }
      vars_constructed[var.storage_index] = true;
   }
   return ret;
}

}} //namespace lofty::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _pvt {

/*static*/ enum_member const * enum_member::find_in_map(enum_member const * members, int value) {
   for (; members->name; ++members) {
      if (value == members->value) {
         return members;
      }
   }
   // TODO: provide more information in the exception.
   LOFTY_THROW(domain_error, ());
}
/*static*/ enum_member const * enum_member::find_in_map(enum_member const * members, str const & name) {
   for (; members->name; ++members) {
      if (name == str(external_buffer, members->name, members->name_size)) {
         return members;
      }
   }
   // TODO: provide more information in the exception.
   LOFTY_THROW(domain_error, ());
}

}} //namespace lofty::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _pvt {

void enum_to_text_ostream_impl::set_format(str const & format) {
   auto itr(format.cbegin());

   // Add parsing of the format string here.

   throw_on_unused_streaming_format_chars(itr, format);
}

void enum_to_text_ostream_impl::write_impl(int i, enum_member const * members, io::text::ostream * dst) {
   auto member = enum_member::find_in_map(members, i);
   dst->write(str(external_buffer, member->name));
}

}} //namespace lofty::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _pvt {

struct from_str_helper::impl {
   text::parsers::regex_capture_format format;
   text::parsers::dynamic parser;
   text::parsers::dynamic::match match;
   text::parsers::dynamic_match_capture t_capture;
};


from_str_helper::from_str_helper() :
   pimpl(new impl()),
   parser(&pimpl->parser) {
}

from_str_helper::~from_str_helper() {
}

text::parsers::dynamic_match_capture const & from_str_helper::parse_src(
   str const & src, text::parsers::dynamic_state const * t_first_state
) {
   auto end_state = parser->create_end_state();
   auto t_cap_state = parser->create_capture_group(t_first_state);
   t_cap_state->set_next(end_state);
   auto begin_state = parser->create_begin_state();
   begin_state->set_next(t_cap_state);
   parser->set_initial_state(begin_state);

   pimpl->match = parser->run(src);
   if (!pimpl->match) {
      /* TODO: with a fair bit of work, parsers::dynamic could be modified to track the farthest code point
      index it could parse successfully. */
      LOFTY_THROW(text::syntax_error, (LOFTY_SL("malformed input"), src, 0));
   }
   pimpl->t_capture = pimpl->match.capture_group(0);
   return pimpl->t_capture;
}

text::parsers::regex_capture_format const & from_str_helper::parse_format_expr(str const & format_expr) {
   // TODO: parse format_expr with regex::parse_capture_format() (itself a TODO).
   pimpl->format.expr = str(external_buffer, format_expr.data(), format_expr.size());
   return pimpl->format;
}

}} //namespace lofty::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace logging { namespace _pvt {

coroutine_local_value<scope_trace const *> scope_trace::scope_traces_head /*= nullptr*/;
coroutine_local_ptr<io::text::str_ostream> scope_trace::trace_ostream;
coroutine_local_value<unsigned> scope_trace::trace_ostream_refs /*= 0*/;
coroutine_local_value<unsigned> scope_trace::curr_stack_depth /*= 0*/;

scope_trace::scope_trace(source_file_address const * source_file_addr_, void const * local_this_) :
   prev_scope_trace(scope_traces_head),
   source_file_addr(source_file_addr_),
   local_this(local_this_) {
   scope_traces_head = this;
}

scope_trace::~scope_trace() {
   if (_std::uncaught_exception()) {
      try {
         write(get_trace_ostream(), ++curr_stack_depth);
      } catch (...) {
         // Don’t allow a trace to interfere with the program flow.
         // FIXME: EXC-SWALLOW
      }
   }
   // Restore the previous scope trace single-linked list head.
   scope_traces_head = prev_scope_trace;
}

void scope_trace::write(io::text::ostream * dst, unsigned stack_depth) const {
   dst->print(
      LOFTY_SL("#{} {} this={} at {}\n"),
      stack_depth, str(external_buffer, source_file_addr->function()), local_this,
      source_file_addr->file_address()
   );
}

/*static*/ void scope_trace::write_list(io::text::ostream * dst) {
   unsigned stack_depth = curr_stack_depth;
   for (scope_trace const * st = scope_traces_head; st; st = st->prev_scope_trace) {
      st->write(dst, ++stack_depth);
   }
}

}}} //namespace lofty::logging::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace bitmanip { namespace _pvt {

std::uint8_t ceiling_to_pow2(std::uint8_t i) {
   unsigned ret = static_cast<unsigned>(i - 1);
   ret |= ret >> 1;
   ret |= ret >> 2;
   ret |= ret >> 4;
   return static_cast<std::uint8_t>(ret + 1);
}
std::uint16_t ceiling_to_pow2(std::uint16_t i) {
   unsigned ret = static_cast<unsigned>(i - 1);
   ret |= ret >> 1;
   ret |= ret >> 2;
   ret |= ret >> 4;
   ret |= ret >> 8;
   return static_cast<std::uint16_t>(ret + 1);
}
std::uint32_t ceiling_to_pow2(std::uint32_t i) {
   --i;
   i |= i >> 1;
   i |= i >> 2;
   i |= i >> 4;
   i |= i >> 8;
   i |= i >> 16;
   return i + 1;
}
std::uint64_t ceiling_to_pow2(std::uint64_t i) {
   --i;
   i |= i >> 1;
   i |= i >> 2;
   i |= i >> 4;
   i |= i >> 8;
   i |= i >> 16;
   i |= i >> 32;
   return i + 1;
}

}}} //namespace lofty::bitmanip::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef LOFTY_HAVE_BSWAP

namespace lofty { namespace byte_order { namespace _pvt {

std::uint16_t bswap_16(std::uint16_t i) {
   return std::uint16_t(
      ((i & std::uint16_t(0xff00u)) >> 8) |
      ((i & std::uint16_t(0x00ffu)) << 8)
   );
}

std::uint32_t bswap_32(std::uint32_t i) {
   return std::uint32_t(
      ((i & std::uint32_t(0xff000000u)) >> 24) |
      ((i & std::uint32_t(0x00ff0000u)) >>  8) |
      ((i & std::uint32_t(0x0000ff00u)) <<  8) |
      ((i & std::uint32_t(0x000000ffu)) << 24)
   );
}

std::uint64_t bswap_64(std::uint64_t i) {
   return std::uint64_t(
      ((i & std::uint64_t(0xff00000000000000u)) >> 56) |
      ((i & std::uint64_t(0x00ff000000000000u)) >> 40) |
      ((i & std::uint64_t(0x0000ff0000000000u)) >> 24) |
      ((i & std::uint64_t(0x000000ff00000000u)) >>  8) |
      ((i & std::uint64_t(0x00000000ff000000u)) <<  8) |
      ((i & std::uint64_t(0x0000000000ff0000u)) << 24) |
      ((i & std::uint64_t(0x000000000000ff00u)) << 40) |
      ((i & std::uint64_t(0x00000000000000ffu)) << 56)
   );
}

}}} //namespace lofty::byte_order::_pvt

#endif //ifndef LOFTY_HAVE_BSWAP

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace logging {

io::text::ostream * get_ostream_if(level level_) {
   // TODO: actually check level_ against a global log level setting.
   return io::text::stderr.get();
}

}} //namespace lofty::logging

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace math {

/*explicit*/ arithmetic_error::arithmetic_error(errint_t err_ /*= 0*/) :
   generic_error(err_) {
}

arithmetic_error::arithmetic_error(arithmetic_error const & src) :
   generic_error(src) {
}

/*virtual*/ arithmetic_error::~arithmetic_error() LOFTY_STL_NOEXCEPT_TRUE() {
}

arithmetic_error & arithmetic_error::operator=(arithmetic_error const & src) {
   generic_error::operator=(src);
   return *this;
}

}} //namespace lofty::math

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace math {

/*explicit*/ division_by_zero::division_by_zero(errint_t err_ /*= 0*/) :
   arithmetic_error(err_) {
}

division_by_zero::division_by_zero(division_by_zero const & src) :
   arithmetic_error(src) {
}

/*virtual*/ division_by_zero::~division_by_zero() LOFTY_STL_NOEXCEPT_TRUE() {
}

division_by_zero & division_by_zero::operator=(division_by_zero const & src) {
   arithmetic_error::operator=(src);
   return *this;
}

}} //namespace lofty::math

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace math {

/*explicit*/ floating_point_error::floating_point_error(errint_t err_ /*= 0*/) :
   arithmetic_error(err_) {
}

floating_point_error::floating_point_error(floating_point_error const & src) :
   arithmetic_error(src) {
}

/*virtual*/ floating_point_error::~floating_point_error() LOFTY_STL_NOEXCEPT_TRUE() {
}

floating_point_error & floating_point_error::operator=(floating_point_error const & src) {
   arithmetic_error::operator=(src);
   return *this;
}

}} //namespace lofty::math

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace math {

/*explicit*/ overflow::overflow(errint_t err_ /*= 0*/) :
   arithmetic_error(err_ ? err_ :
#if LOFTY_HOST_API_POSIX
      EOVERFLOW
#else
      0
#endif
   ) {
}

overflow::overflow(overflow const & src) :
   arithmetic_error(src) {
}

/*virtual*/ overflow::~overflow() LOFTY_STL_NOEXCEPT_TRUE() {
}

overflow & overflow::operator=(overflow const & src) {
   arithmetic_error::operator=(src);
   return *this;
}

}} //namespace lofty::math
