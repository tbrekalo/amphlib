#include "tbrekalo/library.h"

#include <sqlite3.h>

#include <memory>
#include <stdexcept>

namespace tbrekalo::sql {

static constexpr auto CREATE_BOOK_TBL = R"(
  CREATE TABLE IF NOT EXISTS book(
    uuid TEXT PRIMARY KEY,
    isbn TEXT NOT NULL,
    name TEXT NOT NULL,
    authors TEXT NOT NULL,
    borrow INTEGER DEFAULT 0
  );
)";

};

namespace tbrekalo {

using unique_sqlite3 =
    std::unique_ptr<sqlite3,
                    decltype([](sqlite3* db) -> void { sqlite3_close(db); })>;

struct Library::Impl {
  unique_sqlite3 db;
};

Library::Library(std::string_view path) {
  sqlite3* db;
  if (sqlite3_open(path.data(), &db)) {
    std::runtime_error(sqlite3_errmsg(db));
  }
}

auto make_library(std::string_view path) -> std::unique_ptr<Library> {
  return std::unique_ptr<Library>(new Library(path));
}

Library::~Library() {}

auto Library::insert(Book const& book) -> UUID {
  UUID uuid;
  return uuid;
}

auto Library::erase(UUID uuid) -> void {

}

}  // namespace tbrekalo
