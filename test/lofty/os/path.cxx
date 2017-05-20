/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2011-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>
#include <lofty/os/path.hxx>
#include <lofty/testing/test_case.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   os_path_relative_and_absolute_normalization,
   "lofty::os::path – normalization of relative and absolute paths"
) {
   LOFTY_TRACE_FUNC(this);

   /* Note that under Win32, paths that start with “/” are still relative to the current volume; nonetheless,
   the assertions should still be valid. */

   str const sep(os::path::separator());
   text::sstr<64> formatted;
#define norm_path(s)   str(os::path(LOFTY_SL(s)).normalize())
#define format_seps(s) (formatted.format(LOFTY_SL(s), sep), formatted)

   // Empty path.
   LOFTY_TESTING_ASSERT_EQUAL(norm_path(""),          format_seps("")            );
   // Separator only.
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("/"),         format_seps("{0}")         );

   // One component, no separators.
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("."),         format_seps("")            );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path(".."),        format_seps("")            );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("..."),       format_seps("...")         );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("a"),         format_seps("a")           );
   // One component, leading separator.
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("/."),        format_seps("{0}")         );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("/.."),       format_seps("{0}")         );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("/..."),      format_seps("{0}...")      );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("/a"),        format_seps("{0}a")        );
   // One component, trailing separator.
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("./"),        format_seps("")            );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("../"),       format_seps("")            );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path(".../"),      format_seps("...")         );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("a/"),        format_seps("a")           );
   // One component, leading and trailing separators.
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("/./"),       format_seps("{0}")         );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("/../"),      format_seps("{0}")         );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("/.../"),     format_seps("{0}...")      );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("/a/"),       format_seps("{0}a")        );

   // Two components, no separators.
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("./."),       format_seps("")            );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("./.."),      format_seps("")            );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("./..."),     format_seps("...")         );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("./a"),       format_seps("a")           );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("../."),      format_seps("")            );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("../.."),     format_seps("")            );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("../..."),    format_seps("...")         );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("../a"),      format_seps("a")           );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path(".../."),     format_seps("...")         );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path(".../.."),    format_seps("")            );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path(".../..."),   format_seps("...{0}...")   );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path(".../a"),     format_seps("...{0}a")     );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("a/."),       format_seps("a")           );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("a/.."),      format_seps("")            );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("a/..."),     format_seps("a{0}...")     );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("a/a"),       format_seps("a{0}a")       );
   // Two components, leading separator.
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("/./."),      format_seps("{0}")         );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("/./.."),     format_seps("{0}")         );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("/./..."),    format_seps("{0}...")      );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("/./a"),      format_seps("{0}a")        );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("/../."),     format_seps("{0}")         );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("/../.."),    format_seps("{0}")         );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("/../..."),   format_seps("{0}...")      );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("/../a"),     format_seps("{0}a")        );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("/.../."),    format_seps("{0}...")      );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("/.../.."),   format_seps("{0}")         );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("/.../..."),  format_seps("{0}...{0}..."));
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("/.../a"),    format_seps("{0}...{0}a")  );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("/a/."),      format_seps("{0}a")        );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("/a/.."),     format_seps("{0}")         );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("/a/..."),    format_seps("{0}a{0}...")  );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("/a/a"),      format_seps("{0}a{0}a")    );
   // Two components, trailing separator.
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("././"),      format_seps("")            );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("./../"),     format_seps("")            );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("./.../"),    format_seps("...")         );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("./a/"),      format_seps("a")           );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path(".././"),     format_seps("")            );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("../../"),    format_seps("")            );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("../.../"),   format_seps("...")         );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("../a/"),     format_seps("a")           );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("..././"),    format_seps("...")         );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path(".../../"),   format_seps("")            );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path(".../.../"),  format_seps("...{0}...")   );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path(".../a/"),    format_seps("...{0}a")     );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("a/./"),      format_seps("a")           );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("a/../"),     format_seps("")            );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("a/.../"),    format_seps("a{0}...")     );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("a/a/"),      format_seps("a{0}a")       );
   // Two components, leading and trailing separators.
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("/././"),     format_seps("{0}")         );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("/./../"),    format_seps("{0}")         );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("/./.../"),   format_seps("{0}...")      );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("/./a/"),     format_seps("{0}a")        );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("/.././"),    format_seps("{0}")         );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("/../../"),   format_seps("{0}")         );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("/../.../"),  format_seps("{0}...")      );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("/../a/"),    format_seps("{0}a")        );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("/..././"),   format_seps("{0}...")      );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("/.../../"),  format_seps("{0}")         );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("/.../.../"), format_seps("{0}...{0}..."));
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("/.../a/"),   format_seps("{0}...{0}a")  );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("/a/./"),     format_seps("{0}a")        );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("/a/../"),    format_seps("{0}")         );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("/a/.../"),   format_seps("{0}a{0}...")  );
   LOFTY_TESTING_ASSERT_EQUAL(norm_path("/a/a/"),     format_seps("{0}a{0}a")    );

#undef format_seps
#undef norm_path
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   os_path_joined_normalization,
   "lofty::os::path – normalization of joined paths"
) {
   LOFTY_TRACE_FUNC(this);

   os::path path(os::path::current_dir());

   // These should be normalized out.
   LOFTY_TESTING_ASSERT_EQUAL((path / LOFTY_SL(""   )).normalize(), path);
   LOFTY_TESTING_ASSERT_EQUAL((path / LOFTY_SL("/"  )).normalize(), path);
   LOFTY_TESTING_ASSERT_EQUAL((path / LOFTY_SL("//" )).normalize(), path);
   LOFTY_TESTING_ASSERT_EQUAL((path / LOFTY_SL("."  )).normalize(), path);
   LOFTY_TESTING_ASSERT_EQUAL((path / LOFTY_SL("/." )).normalize(), path);
   LOFTY_TESTING_ASSERT_EQUAL((path / LOFTY_SL("./" )).normalize(), path);
   LOFTY_TESTING_ASSERT_EQUAL((path / LOFTY_SL("/./")).normalize(), path);
   LOFTY_TESTING_ASSERT_EQUAL((path / LOFTY_SL("./.")).normalize(), path);

   // These should NOT be normalized: three dots are just another regular path component.
   LOFTY_TESTING_ASSERT_NOT_EQUAL((path / LOFTY_SL("..."  )).normalize(), path);
   LOFTY_TESTING_ASSERT_NOT_EQUAL((path / LOFTY_SL("/..." )).normalize(), path);
   LOFTY_TESTING_ASSERT_NOT_EQUAL((path / LOFTY_SL(".../" )).normalize(), path);
   LOFTY_TESTING_ASSERT_NOT_EQUAL((path / LOFTY_SL("/.../")).normalize(), path);

   // Now with one additional trailing component.
   LOFTY_TESTING_ASSERT_EQUAL((path / LOFTY_SL("/test"   )).normalize(), path / LOFTY_SL("test"));
   LOFTY_TESTING_ASSERT_EQUAL((path / LOFTY_SL("//test"  )).normalize(), path / LOFTY_SL("test"));
   LOFTY_TESTING_ASSERT_EQUAL((path / LOFTY_SL("./test"  )).normalize(), path / LOFTY_SL("test"));
   LOFTY_TESTING_ASSERT_EQUAL((path / LOFTY_SL("/./test" )).normalize(), path / LOFTY_SL("test"));
   LOFTY_TESTING_ASSERT_EQUAL((path / LOFTY_SL("././test")).normalize(), path / LOFTY_SL("test"));

   // Verify that ".." works.
   LOFTY_TESTING_ASSERT_EQUAL((path / LOFTY_SL("a/.."       )).normalize(), path);
   LOFTY_TESTING_ASSERT_EQUAL((path / LOFTY_SL("a/../b"     )).normalize(), path / LOFTY_SL("b"));
   LOFTY_TESTING_ASSERT_EQUAL((path / LOFTY_SL("a/../b/.."  )).normalize(), path);
   LOFTY_TESTING_ASSERT_EQUAL((path / LOFTY_SL("a/b/../.."  )).normalize(), path);
   LOFTY_TESTING_ASSERT_EQUAL((path / LOFTY_SL("a/b/../c"   )).normalize(), path / LOFTY_SL("a/c"));
   LOFTY_TESTING_ASSERT_EQUAL((path / LOFTY_SL("a/../b/../c")).normalize(), path / LOFTY_SL("c"));
   LOFTY_TESTING_ASSERT_EQUAL((path / LOFTY_SL("a/b/../../c")).normalize(), path / LOFTY_SL("c"));
}

}} //namespace lofty::test
