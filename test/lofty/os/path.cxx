/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2011-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>
#include <lofty/logging.hxx>
#include <lofty/os/path.hxx>
#include <lofty/testing/test_case.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   os_path_relative_and_absolute_normalization,
   "lofty::os::path – normalization of relative and absolute paths"
) {
   LOFTY_TRACE_FUNC();

   /* Note that under Win32, paths that start with “/” are still relative to the current volume; nonetheless,
   the assertions should still be valid. */

   str const sep(os::path::separator());
   text::sstr<64> formatted;
#define norm_path(s)   str(os::path(LOFTY_SL(s)).normalize())
#define format_seps(s) (formatted.format(LOFTY_SL(s), sep), formatted)

   // Empty path.
   ASSERT(norm_path("")          == format_seps("")            );
   // Separator only.
   ASSERT(norm_path("/")         == format_seps("{0}")         );

   // One component, no separators.
   ASSERT(norm_path(".")         == format_seps("")            );
   ASSERT(norm_path("..")        == format_seps("")            );
   ASSERT(norm_path("...")       == format_seps("...")         );
   ASSERT(norm_path("a")         == format_seps("a")           );
   // One component, leading separator.
   ASSERT(norm_path("/.")        == format_seps("{0}")         );
   ASSERT(norm_path("/..")       == format_seps("{0}")         );
   ASSERT(norm_path("/...")      == format_seps("{0}...")      );
   ASSERT(norm_path("/a")        == format_seps("{0}a")        );
   // One component, trailing separator.
   ASSERT(norm_path("./")        == format_seps("")            );
   ASSERT(norm_path("../")       == format_seps("")            );
   ASSERT(norm_path(".../")      == format_seps("...")         );
   ASSERT(norm_path("a/")        == format_seps("a")           );
   // One component, leading and trailing separators.
   ASSERT(norm_path("/./")       == format_seps("{0}")         );
   ASSERT(norm_path("/../")      == format_seps("{0}")         );
   ASSERT(norm_path("/.../")     == format_seps("{0}...")      );
   ASSERT(norm_path("/a/")       == format_seps("{0}a")        );

   // Two components, no separators.
   ASSERT(norm_path("./.")       == format_seps("")            );
   ASSERT(norm_path("./..")      == format_seps("")            );
   ASSERT(norm_path("./...")     == format_seps("...")         );
   ASSERT(norm_path("./a")       == format_seps("a")           );
   ASSERT(norm_path("../.")      == format_seps("")            );
   ASSERT(norm_path("../..")     == format_seps("")            );
   ASSERT(norm_path("../...")    == format_seps("...")         );
   ASSERT(norm_path("../a")      == format_seps("a")           );
   ASSERT(norm_path(".../.")     == format_seps("...")         );
   ASSERT(norm_path(".../..")    == format_seps("")            );
   ASSERT(norm_path(".../...")   == format_seps("...{0}...")   );
   ASSERT(norm_path(".../a")     == format_seps("...{0}a")     );
   ASSERT(norm_path("a/.")       == format_seps("a")           );
   ASSERT(norm_path("a/..")      == format_seps("")            );
   ASSERT(norm_path("a/...")     == format_seps("a{0}...")     );
   ASSERT(norm_path("a/a")       == format_seps("a{0}a")       );
   // Two components, leading separator.
   ASSERT(norm_path("/./.")      == format_seps("{0}")         );
   ASSERT(norm_path("/./..")     == format_seps("{0}")         );
   ASSERT(norm_path("/./...")    == format_seps("{0}...")      );
   ASSERT(norm_path("/./a")      == format_seps("{0}a")        );
   ASSERT(norm_path("/../.")     == format_seps("{0}")         );
   ASSERT(norm_path("/../..")    == format_seps("{0}")         );
   ASSERT(norm_path("/../...")   == format_seps("{0}...")      );
   ASSERT(norm_path("/../a")     == format_seps("{0}a")        );
   ASSERT(norm_path("/.../.")    == format_seps("{0}...")      );
   ASSERT(norm_path("/.../..")   == format_seps("{0}")         );
   ASSERT(norm_path("/.../...")  == format_seps("{0}...{0}..."));
   ASSERT(norm_path("/.../a")    == format_seps("{0}...{0}a")  );
   ASSERT(norm_path("/a/.")      == format_seps("{0}a")        );
   ASSERT(norm_path("/a/..")     == format_seps("{0}")         );
   ASSERT(norm_path("/a/...")    == format_seps("{0}a{0}...")  );
   ASSERT(norm_path("/a/a")      == format_seps("{0}a{0}a")    );
   // Two components, trailing separator.
   ASSERT(norm_path("././")      == format_seps("")            );
   ASSERT(norm_path("./../")     == format_seps("")            );
   ASSERT(norm_path("./.../")    == format_seps("...")         );
   ASSERT(norm_path("./a/")      == format_seps("a")           );
   ASSERT(norm_path(".././")     == format_seps("")            );
   ASSERT(norm_path("../../")    == format_seps("")            );
   ASSERT(norm_path("../.../")   == format_seps("...")         );
   ASSERT(norm_path("../a/")     == format_seps("a")           );
   ASSERT(norm_path("..././")    == format_seps("...")         );
   ASSERT(norm_path(".../../")   == format_seps("")            );
   ASSERT(norm_path(".../.../")  == format_seps("...{0}...")   );
   ASSERT(norm_path(".../a/")    == format_seps("...{0}a")     );
   ASSERT(norm_path("a/./")      == format_seps("a")           );
   ASSERT(norm_path("a/../")     == format_seps("")            );
   ASSERT(norm_path("a/.../")    == format_seps("a{0}...")     );
   ASSERT(norm_path("a/a/")      == format_seps("a{0}a")       );
   // Two components, leading and trailing separators.
   ASSERT(norm_path("/././")     == format_seps("{0}")         );
   ASSERT(norm_path("/./../")    == format_seps("{0}")         );
   ASSERT(norm_path("/./.../")   == format_seps("{0}...")      );
   ASSERT(norm_path("/./a/")     == format_seps("{0}a")        );
   ASSERT(norm_path("/.././")    == format_seps("{0}")         );
   ASSERT(norm_path("/../../")   == format_seps("{0}")         );
   ASSERT(norm_path("/../.../")  == format_seps("{0}...")      );
   ASSERT(norm_path("/../a/")    == format_seps("{0}a")        );
   ASSERT(norm_path("/..././")   == format_seps("{0}...")      );
   ASSERT(norm_path("/.../../")  == format_seps("{0}")         );
   ASSERT(norm_path("/.../.../") == format_seps("{0}...{0}..."));
   ASSERT(norm_path("/.../a/")   == format_seps("{0}...{0}a")  );
   ASSERT(norm_path("/a/./")     == format_seps("{0}a")        );
   ASSERT(norm_path("/a/../")    == format_seps("{0}")         );
   ASSERT(norm_path("/a/.../")   == format_seps("{0}a{0}...")  );
   ASSERT(norm_path("/a/a/")     == format_seps("{0}a{0}a")    );

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
   LOFTY_TRACE_FUNC();

   os::path path(os::path::current_dir());

   // These should be normalized out.
   ASSERT((path / LOFTY_SL(""   )).normalize() == path);
   ASSERT((path / LOFTY_SL("/"  )).normalize() == path);
   ASSERT((path / LOFTY_SL("//" )).normalize() == path);
   ASSERT((path / LOFTY_SL("."  )).normalize() == path);
   ASSERT((path / LOFTY_SL("/." )).normalize() == path);
   ASSERT((path / LOFTY_SL("./" )).normalize() == path);
   ASSERT((path / LOFTY_SL("/./")).normalize() == path);
   ASSERT((path / LOFTY_SL("./.")).normalize() == path);

   // These should NOT be normalized: three dots are just another regular path component.
   ASSERT((path / LOFTY_SL("..."  )).normalize() != path);
   ASSERT((path / LOFTY_SL("/..." )).normalize() != path);
   ASSERT((path / LOFTY_SL(".../" )).normalize() != path);
   ASSERT((path / LOFTY_SL("/.../")).normalize() != path);

   // Now with one additional trailing component.
   ASSERT((path / LOFTY_SL("/test"   )).normalize() == path / LOFTY_SL("test"));
   ASSERT((path / LOFTY_SL("//test"  )).normalize() == path / LOFTY_SL("test"));
   ASSERT((path / LOFTY_SL("./test"  )).normalize() == path / LOFTY_SL("test"));
   ASSERT((path / LOFTY_SL("/./test" )).normalize() == path / LOFTY_SL("test"));
   ASSERT((path / LOFTY_SL("././test")).normalize() == path / LOFTY_SL("test"));

   // Verify that ".." works.
   ASSERT((path / LOFTY_SL("a/.."       )).normalize() == path);
   ASSERT((path / LOFTY_SL("a/../b"     )).normalize() == path / LOFTY_SL("b"));
   ASSERT((path / LOFTY_SL("a/../b/.."  )).normalize() == path);
   ASSERT((path / LOFTY_SL("a/b/../.."  )).normalize() == path);
   ASSERT((path / LOFTY_SL("a/b/../c"   )).normalize() == path / LOFTY_SL("a/c"));
   ASSERT((path / LOFTY_SL("a/../b/../c")).normalize() == path / LOFTY_SL("c"));
   ASSERT((path / LOFTY_SL("a/b/../../c")).normalize() == path / LOFTY_SL("c"));
}

}} //namespace lofty::test
