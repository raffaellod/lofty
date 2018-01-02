# -*- coding: utf-8; tab-width: 3; indent-tabs-mode: nil -*-
#
# Copyright 2017 Brent Dimmig
# Copyright 2017-2018 Raffaello D. Di Napoli
#
# This file is part of Lofty.
#
# Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
# Lesser General Public License as published by the Free Software Foundation.
#
# Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
# warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
# for more details.
#-------------------------------------------------------------------------------------------------------------
project(lofty)

cmake_minimum_required(VERSION 2.8.12)
add_definitions("-std=c++11 -fnon-call-exceptions -fvisibility=hidden -fdiagnostics-color=always -fPIC -ggdb -O0 -Wall -Wextra -pedantic -Wconversion -Wlogical-op -Wmissing-declarations -Wpacked -Wshadow -Wsign-conversion -Wundef")

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

include_directories(include)

add_library(lofty
   src/lofty/app.cxx
   src/lofty/collections.cxx
   src/lofty/collections/_pvt/hash_map_impl.cxx
   src/lofty/collections/_pvt/trie_ordered_multimap_impl.cxx
   src/lofty/collections/_pvt/vextr_impl.cxx
   src/lofty/coroutine.cxx
   src/lofty/exception.cxx
   src/lofty/exception-throw_os_error.cxx
   src/lofty/from_text_istream.cxx
   src/lofty/io.cxx
   src/lofty/io/binary.cxx
   src/lofty/io/binary/default_buffered.cxx
   src/lofty/io/binary/file-subclasses.cxx
   src/lofty/io/text.cxx
   src/lofty/io/text/binbuf.cxx
   src/lofty/io/text/str.cxx
   src/lofty/lofty.cxx
   src/lofty/memory.cxx
   src/lofty/net.cxx
   src/lofty/net/tcp.cxx
   src/lofty/net/udp.cxx
   src/lofty/os.cxx
   src/lofty/os/path.cxx
   src/lofty/perf/stopwatch.cxx
   src/lofty/process.cxx
   src/lofty/_pvt/signal_dispatcher.cxx
   src/lofty/_std.cxx
   src/lofty/text.cxx
   src/lofty/text/char_ptr_to_str_adapter.cxx
   src/lofty/text/char_traits.cxx
   src/lofty/text/parsers/ansi_escape_sequences.cxx
   src/lofty/text/parsers/dynamic.cxx
   src/lofty/text/parsers/regex.cxx
   src/lofty/text/str.cxx
   src/lofty/text/str_traits.cxx
   src/lofty/text/ucd.cxx
   src/lofty/thread.cxx
   src/lofty/to_text_ostream.cxx
)
target_link_libraries(lofty dl pthread)

add_library(lofty-testing
   src/lofty-testing/lofty-testing.cxx
)
target_link_libraries(lofty-testing lofty)

add_executable(lofty-test
   test/lofty/collections/hash_map.cxx
   test/lofty/collections/list.cxx
   test/lofty/collections/queue.cxx
   test/lofty/collections/static_list.cxx
   test/lofty/collections/trie_ordered_multimap.cxx
   test/lofty/collections/vector.cxx
   test/lofty/coroutine.cxx
   test/lofty/exception.cxx
   test/lofty/from_text_istream.cxx
   test/lofty/io/text/binbuf_istream-read.cxx
   test/lofty/io/text/istream-scan.cxx
   test/lofty/io/text/ostream-print.cxx
   test/lofty/lofty-test.cxx
   test/lofty/net.cxx
   test/lofty/os/path.cxx
   test/lofty/process.cxx
   test/lofty/text/parsers/dynamic.cxx
   test/lofty/text/str.cxx
   test/lofty/text/str_traits.cxx
   test/lofty/thread.cxx
   test/lofty/to_text_ostream.cxx
)
target_link_libraries(lofty-test lofty-testing lofty)

add_executable(hello-world
   examples/hello-world.cxx
)
target_link_libraries(hello-world lofty)

add_executable(cat
   examples/cat.cxx
)
target_link_libraries(cat lofty)

add_executable(exceptions
   examples/exceptions.cxx
)
target_link_libraries(exceptions lofty)

add_executable(coroutines
   examples/coroutines.cxx
)
target_link_libraries(coroutines lofty)

add_executable(echo-server
   examples/echo-server.cxx
)
target_link_libraries(echo-server lofty)

add_executable(http-server
   examples/http-server.cxx
)
target_link_libraries(http-server lofty)

add_executable(maps-comparison
   examples/maps-comparison.cxx
)
target_link_libraries(maps-comparison lofty)

add_executable(udp-echo-server
   examples/udp-echo-server.cxx
)
target_link_libraries(udp-echo-server lofty)

add_executable(udp-client
   examples/udp-client.cxx
)
target_link_libraries(udp-client lofty)

