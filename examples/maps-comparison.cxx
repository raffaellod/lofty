/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014, 2015
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

#include <abaclade.hxx>
#include <abaclade/app.hxx>
#include <abaclade/collections/hash_map.hxx>
#include <abaclade/perf/stopwatch.hxx>
#include <abaclade/range.hxx>

#include <map>
#include <unordered_map>

using namespace abc;


////////////////////////////////////////////////////////////////////////////////////////////////////

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
      ABC_UNUSED_ARG(t);
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

   @param vsArgs
      Arguments that were provided to this program via command line.
   @return
      Return value of this program.
   */
   virtual int main(collections::mvector<str> & vsArgs) override {
      ABC_TRACE_FUNC(this, vsArgs);

      ABC_UNUSED_ARG(vsArgs);
      using _std::get;

      io::text::stdout->print(ABC_SL(
         "                                                 Add   Hit lookup  Miss lookup  [ns]\n"
      ));

      auto rGoodHash(make_range(0, 10000000));
      io::text::stdout->print(ABC_SL("{}, good hash\n"), rGoodHash.size());
      {
         std::map<int, int> m;
         auto ret(run_test(&m, rGoodHash));
         io::text::stdout->print(
            ABC_SL("  std::map                               {:11}  {:11}  {:11}\n"),
            get<0>(ret), get<1>(ret), get<2>(ret)
         );
      }
      {
         std::unordered_map<int, int, std::hash<int>> um;
         auto ret(run_test(&um, rGoodHash));
         io::text::stdout->print(
            ABC_SL("  std::unordered_map                     {:11}  {:11}  {:11}\n"),
            get<0>(ret), get<1>(ret), get<2>(ret)
         );
      }
      {
         abc::collections::hash_map<int, int, std::hash<int>> hm;
         auto ret(run_test(&hm, rGoodHash));
         io::text::stdout->print(
            ABC_SL("  abc::collections::hash_map (nh: {:5}) {:11}  {:11}  {:11}\n"),
            hm.neighborhood_size(), get<0>(ret), get<1>(ret), get<2>(ret)
         );
      }

      auto rPoorHash(make_range(0, 10000));
      io::text::stdout->print(ABC_SL("{}, 100% collisions\n"), rPoorHash.size());
      {
         std::unordered_map<int, int, poor_hash<int>> um;
         auto ret(run_test(&um, rPoorHash));
         io::text::stdout->print(
            ABC_SL("  std::unordered_map                     {:11}  {:11}  {:11}\n"),
            get<0>(ret), get<1>(ret), get<2>(ret)
         );
      }
      {
         abc::collections::hash_map<int, int, poor_hash<int>> hm;
         auto ret(run_test(&hm, rPoorHash));
         io::text::stdout->print(
            ABC_SL("  abc::collections::hash_map (nh: {:5}) {:11}  {:11}  {:11}\n"),
            hm.neighborhood_size(), get<0>(ret), get<1>(ret), get<2>(ret)
         );
      }

      return 0;
   }

private:
   template <typename TMap, typename TRange>
   perf::stopwatch hit_lookup_test(TMap & m, TRange const & r) {
      ABC_TRACE_FUNC(this/*, m, r*/);

      perf::stopwatch sw;
      sw.start();
      ABC_FOR_EACH(auto i, r) {
         // Consume m[i] in some way.
         if (m[i] != i) {
            io::text::stdout->print(ABC_SL("ERROR for i={}\n"), i);
         }
      }
      sw.stop();
      return std::move(sw);
   }

   template <typename TMap, typename TRange>
   perf::stopwatch miss_lookup_test(TMap & m, TRange const & r) {
      ABC_TRACE_FUNC(this/*, m, r*/);

      perf::stopwatch sw;
      auto end(m.end());
      sw.start();
      ABC_FOR_EACH(auto i, r >> *r.end()) {
         // Consume m[i] in some way.
         if (m.find(i) != end) {
            io::text::stdout->print(ABC_SL("ERROR for i={}\n"), i);
         }
      }
      sw.stop();
      return std::move(sw);
   }

   template <typename TValue>
   run_test_ret run_test(std::map<TValue, TValue> * pm, range<TValue> const & r) {
      ABC_TRACE_FUNC(this, pm/*, r*/);

      perf::stopwatch swAdd;
      {
         swAdd.start();
         ABC_FOR_EACH(auto i, r) {
            pm->insert(std::make_pair(i, i));
         }
         swAdd.stop();
      }
      auto swHitLookup(hit_lookup_test(*pm, r));
      auto swMissLookup(miss_lookup_test(*pm, r));

      return run_test_ret(std::move(swAdd), std::move(swHitLookup), std::move(swMissLookup));
   }

   template <typename TValue, typename THash>
   run_test_ret run_test(std::unordered_map<TValue, TValue, THash> * pum, range<TValue> const & r) {
      ABC_TRACE_FUNC(this, pum/*, r*/);

      perf::stopwatch swAdd;
      {
         swAdd.start();
         ABC_FOR_EACH(auto i, r) {
            pum->insert(std::make_pair(i, i));
         }
         swAdd.stop();
      }
      auto swHitLookup(hit_lookup_test(*pum, r));
      auto swMissLookup(miss_lookup_test(*pum, r));

      return run_test_ret(std::move(swAdd), std::move(swHitLookup), std::move(swMissLookup));
   }

   template <typename TValue, typename THash>
   run_test_ret run_test(
      collections::hash_map<TValue, TValue, THash> * phm, range<TValue> const & r
   ) {
      ABC_TRACE_FUNC(this, phm/*, r*/);

      perf::stopwatch swAdd;
      {
         swAdd.start();
         ABC_FOR_EACH(auto i, r) {
            phm->add_or_assign(i, i);
         }
         swAdd.stop();
      }
      auto swHitLookup(hit_lookup_test(*phm, r));
      auto swMissLookup(miss_lookup_test(*phm, r));

      return run_test_ret(std::move(swAdd), std::move(swHitLookup), std::move(swMissLookup));
   }
};

ABC_APP_CLASS(maps_comparison_app)
