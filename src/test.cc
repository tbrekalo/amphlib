#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <uuid/uuid.h>

#include <ranges>
#include <unordered_set>

#include "doctest/doctest.h"
#include "tbrekalo/isbn.h"
#include "tbrekalo/uuid.h"

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

  SUBCASE("hash") {
    REQUIRE_EQ(
        std::unordered_set<tb::ISBN>{
            *tb::make_isbn(valid),
            *tb::make_isbn(valid),
        }
            .size(),
        1uz);
  }
}

TEST_CASE("UUID") {
  uuid_t source;
  uuid_generate(source);

  uuid_string_t truth;
  uuid_unparse(source, truth);

  uuid_string_t target;
  tb::UUID(tb::UUID::SourceSpan(source)).serialize(target);

  for (auto [lhs, rhs] : std::views::zip(truth, target)) {
    CHECK_EQ(lhs, rhs);
  }
}
