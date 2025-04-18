#include "tbrekalo/book.h"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <unordered_set>

#include "doctest/doctest.h"
#include "tbrekalo/isbn.h"
#include "tbrekalo/storage.h"

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

TEST_CASE("Storage") {
  auto storage = tbrekalo::InMemoryStorage();
  tb::Book prodigy{
      .isbn = *tb::make_isbn("9780720611748"),
      .name = {"The Prodigy"},
      .authors = {"Hermann Hesse"},
  };
  tb::Book east{
      .isbn = *tb::make_isbn("9781466835092"sv),
      .name = {"The Journey to the East"},
      .authors = {"Kermann Hesse"},
  };

  CHECK(storage.insert(prodigy) == 1);
  REQUIRE(storage.n_unique() == 1);

  CHECK(storage.insert(prodigy) == 2);
  REQUIRE(storage.n_unique() == 1);

  CHECK(storage.insert(east) == 1);
  REQUIRE(storage.n_unique() == 2);

  CHECK(storage.remove(prodigy.isbn) == 1);
  REQUIRE(storage.n_unique() == 2);

  CHECK(storage.remove(east.isbn) == 0);
  REQUIRE(storage.n_unique() == 1);

  CHECK(storage.remove(prodigy.isbn) == 0);
  REQUIRE(storage.n_unique() == 0);
}
