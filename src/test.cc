#include "tbrekalo/book.h"
#include "tbrekalo/library.h"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <uuid/uuid.h>

#include <ranges>
#include <unordered_set>

#include "doctest/doctest.h"
#include "tbrekalo/isbn.h"
#include "tbrekalo/uuid.h"

namespace tb = tbrekalo;
using namespace std::literals;

static constexpr tb::Book BOOK_HAMLET{
    .isbn = *tb::make_isbn("9788027237142"),
    .name = "Hamlet",
    .author = "William Shakespeare",
};

static constexpr tb::Book BOOK_SIDDHARTHA{
    .isbn = *tb::make_isbn("9781438279336"),
    .name = "Siddhartha",
    .author = "Hermann Hesse",
};

TEST_SUITE("ISBN") {
  constexpr auto VALID_ISBN_STR = "9781466835191";
  TEST_CASE("ISBNIllformed") {
    REQUIRE_EQ(tb::make_isbn("123").error(), tb::ISBN::Error::INVALID_LENGTH);
    REQUIRE_EQ(tb::make_isbn("01234567891234").error(),
               tb::ISBN::Error::INVALID_LENGTH);
    REQUIRE_EQ(tb::make_isbn("a123456789").error(),
               tb::ISBN::Error::INVALID_CHAR);
    REQUIRE_EQ(tb::make_isbn("a123456789123").error(),
               tb::ISBN::Error::INVALID_CHAR);
  }

  TEST_CASE("valid") {
    REQUIRE_EQ(std::string_view(*tb::make_isbn(VALID_ISBN_STR)),
               VALID_ISBN_STR);
  }

  TEST_CASE("hash") {
    REQUIRE_EQ(
        std::unordered_set<tb::ISBN>{
            *tb::make_isbn(VALID_ISBN_STR),
            *tb::make_isbn(VALID_ISBN_STR),
        }
            .size(),
        1uz);
  }
}

TEST_SUITE("UUID") {
  TEST_CASE("UUIDCreate") {
    uuid_t source;
    uuid_generate_random(source);
    tb::UUID const uuid{tb::UUID::SourceSpan(source)};
    for (auto [lhs, rhs] : std::views::zip(source, uuid.data())) {
      CHECK_EQ(lhs, rhs);
    }
  }

  TEST_CASE("UUIDSerialize") {
    using uuid_str_t = char[37];

    uuid_t source;
    uuid_generate_random(source);
    tb::UUID const uuid{tb::UUID::SourceSpan(source)};

    uuid_str_t source_str;
    uuid_unparse(source, source_str);

    uuid_str_t target_str;
    uuid.serialize(target_str);
    for (auto [lhs, rhs] : std::views::zip(source_str, target_str)) {
      CHECK_EQ(lhs, rhs);
    }
  }

  TEST_CASE("UUIDString") {
    static constexpr std::string_view UUID4(
        "d99d53e1-b67c-438b-8420-63766d8f50d0");
    auto uuid = tb::make_uuid_string(UUID4);

    REQUIRE(uuid.has_value());
    CHECK_EQ(std::string_view(*uuid), UUID4);
  }

  TEST_CASE("UUIDStringIllFormed") {
    static constexpr std::string_view UUID4("d99d53e1-b67c-438b-8420");
    REQUIRE(!tb::make_uuid_string(UUID4).has_value());
  }
}

TEST_SUITE("Library") {
  TEST_CASE("CreateLibrary") {
    auto library = tb::make_library(":memory:");
    REQUIRE(library.has_value());

    {
      auto size = library->size();
      REQUIRE(size.has_value());
      CHECK_EQ(*size, 0);
    }

    {
      auto distinct = library->distinct();
      REQUIRE(distinct.has_value());
      CHECK_EQ(*distinct, 0);
    }
  }

  TEST_CASE("LibraryInsert") {
    auto library = tb::make_library(":memory:");
    REQUIRE(library.has_value());

    auto assert_insertion = [&](tb::Book const& book, std::size_t expected_size,
                                std::size_t expected_distinct) {
      auto result = library->insert(book);
      REQUIRE(result.has_value());

      auto size = library->size();
      REQUIRE(size.has_value());
      CHECK_EQ(*size, expected_size);

      auto distinct = library->distinct();
      REQUIRE(distinct.has_value());
      CHECK_EQ(*distinct, expected_distinct);
    };

    assert_insertion(BOOK_HAMLET, 1, 1);
    assert_insertion(BOOK_HAMLET, 2, 1);
    assert_insertion(BOOK_SIDDHARTHA, 3, 2);
  }

  TEST_CASE("LibraryAuthorLike") {
    auto library = tb::make_library(":memory:");
    REQUIRE(library.has_value());

    auto hamlet_uuid = *library->insert(BOOK_HAMLET);
    auto siddhartha_uuid = *library->insert(BOOK_SIDDHARTHA);

    SUBCASE("FirstName") {
      auto result = library->author_like("William");
      REQUIRE(result.has_value());
    }
  }
}
