/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2011, 2012, 2013, 2014
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
#include <abaclade/testing/test_case.hxx>
#include <abaclade/file_path.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::file_path_normalization

namespace abc {
namespace test {

class file_path_normalization :
   public testing::test_case {
public:

   /** See testing::test_case::title().
   */
   virtual istr title() {
      return istr(SL("abc::file_path - normalization of relative and absolute paths"));
   }


   /** See testing::test_case::run().
   */
   virtual void run() {
      ABC_TRACE_FN((this));

      // Note that under Win32, paths that start with “/” are still relative to the current volume;
      // nonetheless, the assertions should still be valid.

      istr sSep(file_path::separator());
#define norm_path(s)   istr(file_path(SL(s)).normalize())
#define format_seps(s) istr(SL(s)).format(sSep)

      // Empty path.
      ABC_TESTING_ASSERT_EQUAL(norm_path(""),          format_seps("")            );
      // Separator only.
      ABC_TESTING_ASSERT_EQUAL(norm_path("/"),         format_seps("{0}")         );

      // One component, no separators.
      ABC_TESTING_ASSERT_EQUAL(norm_path("."),         format_seps("")            );
      ABC_TESTING_ASSERT_EQUAL(norm_path(".."),        format_seps("")            );
      ABC_TESTING_ASSERT_EQUAL(norm_path("..."),       format_seps("...")         );
      ABC_TESTING_ASSERT_EQUAL(norm_path("a"),         format_seps("a")           );
      // One component, leading separator.
      ABC_TESTING_ASSERT_EQUAL(norm_path("/."),        format_seps("{0}")         );
      ABC_TESTING_ASSERT_EQUAL(norm_path("/.."),       format_seps("{0}")         );
      ABC_TESTING_ASSERT_EQUAL(norm_path("/..."),      format_seps("{0}...")      );
      ABC_TESTING_ASSERT_EQUAL(norm_path("/a"),        format_seps("{0}a")        );
      // One component, trailing separator.
      ABC_TESTING_ASSERT_EQUAL(norm_path("./"),        format_seps("")            );
      ABC_TESTING_ASSERT_EQUAL(norm_path("../"),       format_seps("")            );
      ABC_TESTING_ASSERT_EQUAL(norm_path(".../"),      format_seps("...")         );
      ABC_TESTING_ASSERT_EQUAL(norm_path("a/"),        format_seps("a")           );
      // One component, leading and trailing separators.
      ABC_TESTING_ASSERT_EQUAL(norm_path("/./"),       format_seps("{0}")         );
      ABC_TESTING_ASSERT_EQUAL(norm_path("/../"),      format_seps("{0}")         );
      ABC_TESTING_ASSERT_EQUAL(norm_path("/.../"),     format_seps("{0}...")      );
      ABC_TESTING_ASSERT_EQUAL(norm_path("/a/"),       format_seps("{0}a")        );

      // Two components, no separators.
      ABC_TESTING_ASSERT_EQUAL(norm_path("./."),       format_seps("")            );
      ABC_TESTING_ASSERT_EQUAL(norm_path("./.."),      format_seps("")            );
      ABC_TESTING_ASSERT_EQUAL(norm_path("./..."),     format_seps("...")         );
      ABC_TESTING_ASSERT_EQUAL(norm_path("./a"),       format_seps("a")           );
      ABC_TESTING_ASSERT_EQUAL(norm_path("../."),      format_seps("")            );
      ABC_TESTING_ASSERT_EQUAL(norm_path("../.."),     format_seps("")            );
      ABC_TESTING_ASSERT_EQUAL(norm_path("../..."),    format_seps("...")         );
      ABC_TESTING_ASSERT_EQUAL(norm_path("../a"),      format_seps("a")           );
      ABC_TESTING_ASSERT_EQUAL(norm_path(".../."),     format_seps("...")         );
      ABC_TESTING_ASSERT_EQUAL(norm_path(".../.."),    format_seps("")            );
      ABC_TESTING_ASSERT_EQUAL(norm_path(".../..."),   format_seps("...{0}...")   );
      ABC_TESTING_ASSERT_EQUAL(norm_path(".../a"),     format_seps("...{0}a")     );
      ABC_TESTING_ASSERT_EQUAL(norm_path("a/."),       format_seps("a")           );
      ABC_TESTING_ASSERT_EQUAL(norm_path("a/.."),      format_seps("")            );
      ABC_TESTING_ASSERT_EQUAL(norm_path("a/..."),     format_seps("a{0}...")     );
      ABC_TESTING_ASSERT_EQUAL(norm_path("a/a"),       format_seps("a{0}a")       );
      // Two components, leading separator.
      ABC_TESTING_ASSERT_EQUAL(norm_path("/./."),      format_seps("{0}")         );
      ABC_TESTING_ASSERT_EQUAL(norm_path("/./.."),     format_seps("{0}")         );
      ABC_TESTING_ASSERT_EQUAL(norm_path("/./..."),    format_seps("{0}...")      );
      ABC_TESTING_ASSERT_EQUAL(norm_path("/./a"),      format_seps("{0}a")        );
      ABC_TESTING_ASSERT_EQUAL(norm_path("/../."),     format_seps("{0}")         );
      ABC_TESTING_ASSERT_EQUAL(norm_path("/../.."),    format_seps("{0}")         );
      ABC_TESTING_ASSERT_EQUAL(norm_path("/../..."),   format_seps("{0}...")      );
      ABC_TESTING_ASSERT_EQUAL(norm_path("/../a"),     format_seps("{0}a")        );
      ABC_TESTING_ASSERT_EQUAL(norm_path("/.../."),    format_seps("{0}...")      );
      ABC_TESTING_ASSERT_EQUAL(norm_path("/.../.."),   format_seps("{0}")         );
      ABC_TESTING_ASSERT_EQUAL(norm_path("/.../..."),  format_seps("{0}...{0}..."));
      ABC_TESTING_ASSERT_EQUAL(norm_path("/.../a"),    format_seps("{0}...{0}a")  );
      ABC_TESTING_ASSERT_EQUAL(norm_path("/a/."),      format_seps("{0}a")        );
      ABC_TESTING_ASSERT_EQUAL(norm_path("/a/.."),     format_seps("{0}")         );
      ABC_TESTING_ASSERT_EQUAL(norm_path("/a/..."),    format_seps("{0}a{0}...")  );
      ABC_TESTING_ASSERT_EQUAL(norm_path("/a/a"),      format_seps("{0}a{0}a")    );
      // Two components, trailing separator.
      ABC_TESTING_ASSERT_EQUAL(norm_path("././"),      format_seps("")            );
      ABC_TESTING_ASSERT_EQUAL(norm_path("./../"),     format_seps("")            );
      ABC_TESTING_ASSERT_EQUAL(norm_path("./.../"),    format_seps("...")         );
      ABC_TESTING_ASSERT_EQUAL(norm_path("./a/"),      format_seps("a")           );
      ABC_TESTING_ASSERT_EQUAL(norm_path(".././"),     format_seps("")            );
      ABC_TESTING_ASSERT_EQUAL(norm_path("../../"),    format_seps("")            );
      ABC_TESTING_ASSERT_EQUAL(norm_path("../.../"),   format_seps("...")         );
      ABC_TESTING_ASSERT_EQUAL(norm_path("../a/"),     format_seps("a")           );
      ABC_TESTING_ASSERT_EQUAL(norm_path("..././"),    format_seps("...")         );
      ABC_TESTING_ASSERT_EQUAL(norm_path(".../../"),   format_seps("")            );
      ABC_TESTING_ASSERT_EQUAL(norm_path(".../.../"),  format_seps("...{0}...")   );
      ABC_TESTING_ASSERT_EQUAL(norm_path(".../a/"),    format_seps("...{0}a")     );
      ABC_TESTING_ASSERT_EQUAL(norm_path("a/./"),      format_seps("a")           );
      ABC_TESTING_ASSERT_EQUAL(norm_path("a/../"),     format_seps("")            );
      ABC_TESTING_ASSERT_EQUAL(norm_path("a/.../"),    format_seps("a{0}...")     );
      ABC_TESTING_ASSERT_EQUAL(norm_path("a/a/"),      format_seps("a{0}a")       );
      // Two components, leading and trailing separators.
      ABC_TESTING_ASSERT_EQUAL(norm_path("/././"),     format_seps("{0}")         );
      ABC_TESTING_ASSERT_EQUAL(norm_path("/./../"),    format_seps("{0}")         );
      ABC_TESTING_ASSERT_EQUAL(norm_path("/./.../"),   format_seps("{0}...")      );
      ABC_TESTING_ASSERT_EQUAL(norm_path("/./a/"),     format_seps("{0}a")        );
      ABC_TESTING_ASSERT_EQUAL(norm_path("/.././"),    format_seps("{0}")         );
      ABC_TESTING_ASSERT_EQUAL(norm_path("/../../"),   format_seps("{0}")         );
      ABC_TESTING_ASSERT_EQUAL(norm_path("/../.../"),  format_seps("{0}...")      );
      ABC_TESTING_ASSERT_EQUAL(norm_path("/../a/"),    format_seps("{0}a")        );
      ABC_TESTING_ASSERT_EQUAL(norm_path("/..././"),   format_seps("{0}...")      );
      ABC_TESTING_ASSERT_EQUAL(norm_path("/.../../"),  format_seps("{0}")         );
      ABC_TESTING_ASSERT_EQUAL(norm_path("/.../.../"), format_seps("{0}...{0}..."));
      ABC_TESTING_ASSERT_EQUAL(norm_path("/.../a/"),   format_seps("{0}...{0}a")  );
      ABC_TESTING_ASSERT_EQUAL(norm_path("/a/./"),     format_seps("{0}a")        );
      ABC_TESTING_ASSERT_EQUAL(norm_path("/a/../"),    format_seps("{0}")         );
      ABC_TESTING_ASSERT_EQUAL(norm_path("/a/.../"),   format_seps("{0}a{0}...")  );
      ABC_TESTING_ASSERT_EQUAL(norm_path("/a/a/"),     format_seps("{0}a{0}a")    );

#undef format_seps
#undef norm_path
   }
};

} //namespace test
} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::file_path_normalization)


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::file_path_normalization_joined

namespace abc {
namespace test {

class file_path_normalization_joined :
   public testing::test_case {
public:

   /** See testing::test_case::title().
   */
   virtual istr title() {
      return istr(SL("abc::file_path - normalization of joined paths"));
   }


   /** See testing::test_case::run().
   */
   virtual void run() {
      ABC_TRACE_FN((this));

      file_path fp(file_path::current_dir());

      // These should be normalized out.
      ABC_TESTING_ASSERT_EQUAL((fp / SL(""   )).normalize(), fp);
      ABC_TESTING_ASSERT_EQUAL((fp / SL("/"  )).normalize(), fp);
      ABC_TESTING_ASSERT_EQUAL((fp / SL("//" )).normalize(), fp);
      ABC_TESTING_ASSERT_EQUAL((fp / SL("."  )).normalize(), fp);
      ABC_TESTING_ASSERT_EQUAL((fp / SL("/." )).normalize(), fp);
      ABC_TESTING_ASSERT_EQUAL((fp / SL("./" )).normalize(), fp);
      ABC_TESTING_ASSERT_EQUAL((fp / SL("/./")).normalize(), fp);
      ABC_TESTING_ASSERT_EQUAL((fp / SL("./.")).normalize(), fp);

      // These should NOT be normalized: three dots are just another regular path component.
      ABC_TESTING_ASSERT_NOT_EQUAL((fp / SL("..."  )).normalize(), fp);
      ABC_TESTING_ASSERT_NOT_EQUAL((fp / SL("/..." )).normalize(), fp);
      ABC_TESTING_ASSERT_NOT_EQUAL((fp / SL(".../" )).normalize(), fp);
      ABC_TESTING_ASSERT_NOT_EQUAL((fp / SL("/.../")).normalize(), fp);

      // Now with one additional trailing component.
      ABC_TESTING_ASSERT_EQUAL((fp / SL("/test"   )).normalize(), fp / SL("test"));
      ABC_TESTING_ASSERT_EQUAL((fp / SL("//test"  )).normalize(), fp / SL("test"));
      ABC_TESTING_ASSERT_EQUAL((fp / SL("./test"  )).normalize(), fp / SL("test"));
      ABC_TESTING_ASSERT_EQUAL((fp / SL("/./test" )).normalize(), fp / SL("test"));
      ABC_TESTING_ASSERT_EQUAL((fp / SL("././test")).normalize(), fp / SL("test"));

      // Verify that ".." works.
      ABC_TESTING_ASSERT_EQUAL((fp / SL("a/.."       )).normalize(), fp);
      ABC_TESTING_ASSERT_EQUAL((fp / SL("a/../b"     )).normalize(), fp / SL("b"));
      ABC_TESTING_ASSERT_EQUAL((fp / SL("a/../b/.."  )).normalize(), fp);
      ABC_TESTING_ASSERT_EQUAL((fp / SL("a/b/../.."  )).normalize(), fp);
      ABC_TESTING_ASSERT_EQUAL((fp / SL("a/b/../c"   )).normalize(), fp / SL("a/c"));
      ABC_TESTING_ASSERT_EQUAL((fp / SL("a/../b/../c")).normalize(), fp / SL("c"));
      ABC_TESTING_ASSERT_EQUAL((fp / SL("a/b/../../c")).normalize(), fp / SL("c"));
   }
};

} //namespace test
} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::file_path_normalization_joined)


////////////////////////////////////////////////////////////////////////////////////////////////////

