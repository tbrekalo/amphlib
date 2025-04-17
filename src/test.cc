#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include "tbrekalo/lib.h"

namespace tb = tbrekalo;
using namespace std::literals;

TEST_CASE("ISBN") {
  SUBCASE("illformed") {
    REQUIRE_EQ(tb::make_isbn("123").error(), tb::ISBN::Error::INVALID_LENGTH);
    REQUIRE_EQ(tb::make_isbn("01234567891234").error(),
               tb::ISBN::Error::INVALID_LENGTH);
    REQUIRE_EQ(tb::make_isbn("a123456789").error(),
               tb::ISBN::Error::INVALID_CHAR);
    REQUIRE_EQ(tb::make_isbn("a123456789123").error(),
               tb::ISBN::Error::INVALID_CHAR);
  }

  constexpr auto valid = "9781466835191";
  REQUIRE_EQ(std::string_view(*tb::make_isbn(valid)), valid);
}
