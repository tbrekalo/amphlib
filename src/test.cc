#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include "tbrekalo/lib.h"

TEST_CASE("foo") { CHECK(tbrekalo::foo() == 42); }
