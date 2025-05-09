#include "tbrekalo/uuid.h"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <uuid/uuid.h>

#include <functional>
#include <ranges>
#include <unordered_set>

#include "doctest/doctest.h"
#include "tbrekalo/library.h"

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

  TEST_CASE("UUIDHash") {
    tb::UUID a, b;
    REQUIRE(std::unordered_set<tb::UUID>{a, a, b}.size() == 2);
  }

  TEST_CASE("UUIDString") {
    static constexpr std::string_view UUID4(
        "d99d53e1-b67c-438b-8420-63766d8f50d0");
    auto uuid = tb::make_uuid_string(UUID4);

    REQUIRE(uuid.has_value());
    CHECK_EQ(std::string_view(*uuid), UUID4);
  }

  TEST_CASE("UUIDStringHASH") {
    tb::UUIDString a(tb::UUID{}), b(tb::UUID{});
    REQUIRE(std::unordered_set<tb::UUIDString>{a, a, b}.size() == 2);
  }

  TEST_CASE("UUIDStringIllFormed") {
    static constexpr std::string_view UUID4("d99d53e1-b67c-438b-8420");
    REQUIRE(!tb::make_uuid_string(UUID4).has_value());
  }
}

TEST_SUITE("Library") {
  TEST_CASE("LibraryCreate") {
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

  TEST_CASE("LibraryMove") {
    auto library = *tb::make_library(":memory:");
    auto moved_library(std::move(library));

    {
      auto result = library.insert(BOOK_HAMLET);
      REQUIRE(!result.has_value());
      CHECK_EQ(result.error(), tb::Library::Error::DB_CONNECTION);
    }

    {
      auto result = moved_library.insert(BOOK_HAMLET);
      REQUIRE(result.has_value());
    }

    {
      moved_library = std::move(moved_library);
      auto result = moved_library.size();
      REQUIRE(result.has_value());
      CHECK_EQ(*result, 1);
    }
  }

  TEST_CASE("LibraryInsert") {
    auto library = *tb::make_library(":memory:");

    auto assert_insertion = [&](tb::Book const& book, std::size_t expected_size,
                                std::size_t expected_distinct) {
      auto result = library.insert(book);
      REQUIRE(result.has_value());

      auto size = library.size();
      REQUIRE(size.has_value());
      CHECK_EQ(*size, expected_size);

      auto distinct = library.distinct();
      REQUIRE(distinct.has_value());
      CHECK_EQ(*distinct, expected_distinct);
    };

    assert_insertion(BOOK_HAMLET, 1, 1);
    assert_insertion(BOOK_HAMLET, 2, 1);
    assert_insertion(BOOK_SIDDHARTHA, 3, 2);
  }

  TEST_CASE("LibraryErase") {
    auto library = *tb::make_library(":memory:");
    auto hamlet = *library.insert(BOOK_HAMLET);
    auto omlet = *library.insert(BOOK_HAMLET);

    REQUIRE_EQ(*library.size(), 2);
    REQUIRE_EQ(*library.distinct(), 1);

    {
      auto result = library.erase(omlet);
      REQUIRE(result.has_value());

      CHECK_EQ(*library.size(), 1);
      CHECK_EQ(*library.distinct(), 1);
    }
  }

  TEST_CASE("LibraryRecords") {
    auto library = *tb::make_library(":memory:");
    auto hamlet = *library.insert(BOOK_HAMLET);
    auto omlet = *library.insert(BOOK_HAMLET);
    auto siddhartha = *library.insert(BOOK_SIDDHARTHA);

    CHECK_EQ(*library.size(), 3);
    {
      auto result = library.records();
      REQUIRE(result.has_value());

      std::unordered_set<tb::UUID> uuids, expected{hamlet, omlet, siddhartha};
      for (auto const& record : *result) {
        uuids.insert(record.uuid);
      }

      REQUIRE_EQ(uuids, expected);
    }
  }

  TEST_CASE("LibraryLike") {
    auto library = *tb::make_library(":memory:");

    auto hamlet_uuid = *library.insert(BOOK_HAMLET);
    auto siddhartha_uuid = *library.insert(BOOK_SIDDHARTHA);

    auto make_assert_single = [&library]<class Fn>(Fn&& fn)
      requires(
          std::is_invocable_r_v<std::expected<std::vector<tb::Library::Record>,
                                              tb::Library::Error>,
                                Fn, tb::Library*, std::string_view>)
    {
      return [&library, fn](std::string_view query, tb::UUID expected) {
        auto result = std::invoke(fn, &library, query);
        REQUIRE(result.has_value());
        REQUIRE(result->size() == 1);
        CHECK(result->front().uuid == expected);
      };
    };

    SUBCASE("Author") {
      auto assert_single = make_assert_single(&tb::Library::author_like);
      SUBCASE("FirstName") { assert_single("William", hamlet_uuid); }
      SUBCASE("LaseName") { assert_single("Shakespeare", hamlet_uuid); }
      SUBCASE("Middle") { assert_single("iam Shak", hamlet_uuid); }
    }

    SUBCASE("Name") {
      auto assert_single = make_assert_single(&tb::Library::name_like);
      SUBCASE("Hamlet") { assert_single("aml", hamlet_uuid); }
      SUBCASE("Siddhartha") { assert_single("iddh", siddhartha_uuid); }
    }

    SUBCASE("Duplicate") {
      auto omlet_uuid = *library.insert(BOOK_HAMLET);
      auto result = library.author_like("William");
      REQUIRE(result.has_value());
      REQUIRE(result->size() == 2);

      std::unordered_set<tb::UUID> uuids, expected{hamlet_uuid, omlet_uuid};
      for (auto const& record : *result) {
        uuids.insert(record.uuid);
      }

      REQUIRE_EQ(uuids, expected);
    }
  }

  TEST_CASE("LibraryBorrow") {
    auto library = *tb::make_library(":memory:");
    auto hamlet_uuid = *library.insert(BOOK_HAMLET);

    {
      auto result = library.acquire_book(hamlet_uuid);
      REQUIRE(result.has_value());
    }

    {
      auto result = library.acquire_book(hamlet_uuid);
      REQUIRE(!result.has_value());
    }

    {
      auto result = library.release_book(hamlet_uuid);
      REQUIRE(result.has_value());
    }

    {
      auto result = library.acquire_book(hamlet_uuid);
      REQUIRE(result.has_value());
    }
  }
}
