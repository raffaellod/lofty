/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014-2015, 2017-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty/app.hxx>
#include <lofty/collections/hash_map.hxx>
#include <lofty/collections/vector.hxx>
#include <lofty/io/text.hxx>
#include <lofty/logging.hxx>
#include <lofty/perf/stopwatch.hxx>
#include <lofty/range.hxx>
#include <lofty/_std/functional.hxx>
#include <lofty/_std/tuple.hxx>
#include <lofty/text/str.hxx>
#include <map>
#include <unordered_map>

using namespace lofty;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace {

/*! Inefficient hash functor that results in 100% hash collisions.

@param i
   Value to hash.
@return
   Hash of i.
*/
template <typename T>
struct poor_hash {
   std::size_t operator()(T t) const {
      LOFTY_UNUSED_ARG(t);
      return 0;
   }
};

} //namespace

//! Application class for this program.
class maps_comparison_app : public app {
private:
   typedef _std::tuple<perf::stopwatch, perf::stopwatch, perf::stopwatch> run_test_ret;

public:
   /*! Main function of the program.

   @param args
      Arguments that were provided to this program via command line.
   @return
      Return value of this program.
   */
   virtual int main(collections::vector<text::str> & args) override {
      LOFTY_TRACE_METHOD();

      LOFTY_UNUSED_ARG(args);
      using _std::get;

      io::text::stdout->print(LOFTY_SL(
         "                                                 Add   Hit lookup  Miss lookup  [ns]\n"
      ));

      auto good_hash_range(make_range(0, 10000000));
      io::text::stdout->print(LOFTY_SL("{}, good hash\n"), good_hash_range.size());
      {
         std::map<int, int> m;
         auto ret(run_test(&m, good_hash_range));
         io::text::stdout->print(
            LOFTY_SL("  std::map                               {:11}  {:11}  {:11}\n"),
            get<0>(ret), get<1>(ret), get<2>(ret)
         );
      }
      {
         std::unordered_map<int, int, _std::hash<int>> map;
         auto ret(run_test(&map, good_hash_range));
         io::text::stdout->print(
            LOFTY_SL("  std::unordered_map                     {:11}  {:11}  {:11}\n"),
            get<0>(ret), get<1>(ret), get<2>(ret)
         );
      }
      {
         lofty::collections::hash_map<int, int, _std::hash<int>> map;
         auto ret(run_test(&map, good_hash_range));
         io::text::stdout->print(
            LOFTY_SL("  lofty::collections::hash_map (nh: {:5}) {:11}  {:11}  {:11}\n"),
            map.neighborhood_size(), get<0>(ret), get<1>(ret), get<2>(ret)
         );
      }

      auto poor_hash_range(make_range(0, 10000));
      io::text::stdout->print(LOFTY_SL("{}, 100% collisions\n"), poor_hash_range.size());
      {
         std::unordered_map<int, int, poor_hash<int>> map;
         auto ret(run_test(&map, poor_hash_range));
         io::text::stdout->print(
            LOFTY_SL("  std::unordered_map                     {:11}  {:11}  {:11}\n"),
            get<0>(ret), get<1>(ret), get<2>(ret)
         );
      }
      {
         lofty::collections::hash_map<int, int, poor_hash<int>> map;
         auto ret(run_test(&map, poor_hash_range));
         io::text::stdout->print(
            LOFTY_SL("  lofty::collections::hash_map (nh: {:5}) {:11}  {:11}  {:11}\n"),
            map.neighborhood_size(), get<0>(ret), get<1>(ret), get<2>(ret)
         );
      }

      return 0;
   }

private:
   template <typename TMap, typename TRange>
   perf::stopwatch hit_lookup_test(TMap & map, TRange const & range) {
      LOFTY_TRACE_METHOD();

      perf::stopwatch sw;
      sw.start();
      LOFTY_FOR_EACH(auto i, range) {
         // Consume map[i] in some way.
         if (map[i] != i) {
            io::text::stdout->print(LOFTY_SL("ERROR for i={}\n"), i);
         }
      }
      sw.stop();
      return std::move(sw);
   }

   template <typename TMap, typename TRange>
   perf::stopwatch miss_lookup_test(TMap & map, TRange const & range) {
      LOFTY_TRACE_METHOD();

      perf::stopwatch sw;
      auto end(map.end());
      sw.start();
      LOFTY_FOR_EACH(auto i, range >> *range.end()) {
         // Consume map[i] in some way.
         if (map.find(i) != end) {
            io::text::stdout->print(LOFTY_SL("ERROR for i={}\n"), i);
         }
      }
      sw.stop();
      return std::move(sw);
   }

   template <typename TValue>
   run_test_ret run_test(std::map<TValue, TValue> * map, range<TValue> const & range) {
      LOFTY_TRACE_METHOD();

      perf::stopwatch add_sw;
      {
         add_sw.start();
         LOFTY_FOR_EACH(auto i, range) {
            map->insert(std::make_pair(i, i));
         }
         add_sw.stop();
      }
      auto hit_lookup_sw(hit_lookup_test(*map, range));
      auto miss_lookup_sw(miss_lookup_test(*map, range));

      return run_test_ret(std::move(add_sw), std::move(hit_lookup_sw), std::move(miss_lookup_sw));
   }

   template <typename TValue, typename THash>
   run_test_ret run_test(std::unordered_map<TValue, TValue, THash> * map, range<TValue> const & range) {
      LOFTY_TRACE_METHOD();

      perf::stopwatch add_sw;
      {
         add_sw.start();
         LOFTY_FOR_EACH(auto i, range) {
            map->insert(std::make_pair(i, i));
         }
         add_sw.stop();
      }
      auto hit_lookup_sw(hit_lookup_test(*map, range));
      auto miss_lookup_sw(miss_lookup_test(*map, range));

      return run_test_ret(std::move(add_sw), std::move(hit_lookup_sw), std::move(miss_lookup_sw));
   }

   template <typename TValue, typename THash>
   run_test_ret run_test(collections::hash_map<TValue, TValue, THash> * map, range<TValue> const & range) {
      LOFTY_TRACE_METHOD();

      perf::stopwatch add_sw;
      {
         add_sw.start();
         LOFTY_FOR_EACH(auto i, range) {
            map->add_or_assign(i, i);
         }
         add_sw.stop();
      }
      auto hit_lookup_sw(hit_lookup_test(*map, range));
      auto miss_lookup_sw(miss_lookup_test(*map, range));

      return run_test_ret(std::move(add_sw), std::move(hit_lookup_sw), std::move(miss_lookup_sw));
   }
};

LOFTY_APP_CLASS(maps_comparison_app)
