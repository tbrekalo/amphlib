#include "tbrekalo/library.h"

#include <sqlite3.h>

#include <cassert>
#include <cstddef>
#include <cstdio>
#include <format>
#include <iostream>
#include <memory>
#include <mutex>
#include <source_location>
#include <utility>

#include "tbrekalo/uuid.h"

namespace tbrekalo::meta {

[[noreturn]] void iDoNotExist();
static constexpr struct {
  template <typename T>
  inline consteval operator T() const {
    iDoNotExist();
  }
} REQUIRED;

}  // namespace tbrekalo::meta

namespace tbrekalo::sql {

static constexpr auto INIT_DB_SQL = R"(
  CREATE TABLE IF NOT EXISTS record(
    uuid TEXT PRIMARY KEY,
    isbn TEXT NOT NULL,
    name TEXT NOT NULL,
    author TEXT NOT NULL,
    acquired INTEGER DEFAULT 0
  );
  
  CREATE INDEX IF NOT EXISTS idx_record_name ON record(name);
  CREATE INDEX IF NOT EXISTS idx_record_author ON record(author);
  CREATE INDEX IF NOT EXISTS idx_record_uuid_acquired ON record(uuid, acquired);
)";

static constexpr auto INSERT_FMT =
    R"(INSERT INTO record VALUES('{}', '{}', '{}', '{}', '{}'))";

static auto make_insert_sql(Library::Record const& record) -> std::string {
  return std::format(INSERT_FMT, UUIDString(record.uuid).data(),
                     record.isbn.data(), record.name, record.author,
                     static_cast<int>(record.acquired));
}

static constexpr auto ERASE_FMT = R"(DELETE FROM record WHERE uuid='{}')";

static auto make_erase_sql(UUID uuid) -> std::string {
  return std::format(ERASE_FMT, UUIDString(uuid).data());
}

static constexpr auto COUNT_SQL = R"(SELECT COUNT(*) FROM record;)";

static constexpr auto DISTINCT_SQL =
    R"(SELECT COUNT(DISTINCT isbn) FROM record;)";

static constexpr auto RECORDS_SQL = R"(SELECT * FROM record;)";

static constexpr auto NAME_LIKE_FMT =
    R"(SELECT DISTINCT * FROM record WHERE name LIKE '%{}%';)";

static auto make_name_like_sql(std::string_view name_like) -> std::string {
  return std::format(NAME_LIKE_FMT, name_like);
}

static constexpr auto AUTHOR_LIKE_FMT =
    R"(SELECT DISTINCT * FROM record WHERE author LIKE '%{}%';)";

static auto make_author_like_sql(std::string_view author_like) -> std::string {
  return std::format(AUTHOR_LIKE_FMT, author_like);
}

static constexpr auto ACQUIRE_RECORD_FMT =
    R"(UPDATE record SET acquired=1 WHERE uuid='{}' AND acquired=0;)";

static auto make_acquire_record_sql(UUIDString uuid) -> std::string {
  return std::format(ACQUIRE_RECORD_FMT, static_cast<char const*>(uuid));
}

static constexpr auto RELEASE_RECORD_FMT =
    R"(UPDATE record SET acquired=0 WHERE uuid='{}' AND acquired=1;)";

static auto make_release_record_sql(UUIDString uuid) -> std::string {
  return std::format(RELEASE_RECORD_FMT, static_cast<char const*>(uuid));
}

}  // namespace tbrekalo::sql

namespace tbrekalo {

static auto parse_count(void* count, int n, char** values, char** variables)
    -> int {
  assert(n == 1);
  if (sscanf(values[0], "%zu", static_cast<std::size_t*>(count))) {
    return 0;
  }
  return 1;
}

static auto parse_record(void* records_vec_void_ptr, int n, char** values,
                         char** variables) -> int {
  auto& records =
      *reinterpret_cast<std::vector<Library::Record>*>(records_vec_void_ptr);
  Library::Record record;

  for (int i = 0; i < n; ++i) {
    auto const variable_sv = std::string_view(variables[i]);
    auto const value_sv = std::string_view(values[i]);
    if (variable_sv == "uuid") {
      if (auto opt_uuid = make_uuid_string(value_sv); opt_uuid.has_value()) {
        record.uuid = static_cast<UUID>(UUIDString(*opt_uuid));
        continue;
      }

      return 1;
    }

    if (variable_sv == "isbn") {
      if (auto opt_isbn = make_isbn(value_sv); opt_isbn.has_value()) {
        record.isbn = *opt_isbn;
        continue;
      }

      return 1;
    }

    if (variable_sv == "name") {
      record.name = value_sv;
      continue;
    }

    if (variable_sv == "author") {
      record.author = value_sv;
      continue;
    }

    if (variable_sv == "acquired") {
      if (int x; sscanf(value_sv.data(), "%d", &x)) {
        record.acquired = x;
        continue;
      }
      return 1;
    }

    return 1;
  }

  records.push_back(std::move(record));
  return 0;
}

static auto log(std::string_view message, const std::source_location location =
                                              std::source_location::current()) {
  /* clang-format off */
  std::cerr << std::format("file={}:{} function={} message='{}'",
                           location.file_name(),
                           location.line(),
                           location.function_name(),
                           message) << std::endl;
  /* clang-format on */
}

using unique_sqlite3 =
    std::unique_ptr<sqlite3,
                    decltype([](sqlite3* db) -> void { sqlite3_close(db); })>;

class Library::Impl {
  unique_sqlite3 db_;
  std::mutex db_mutex_;

 public:
  explicit Impl(unique_sqlite3 db) : db_(std::move(db)) {}

  struct ExecuteArgs {
    std::string_view sql = meta::REQUIRED;
    int (*callback)(void*, int, char**, char**) = nullptr;
    void* callback_arg = nullptr;
  };

  auto execute(ExecuteArgs args) -> std::expected<int, Error> {
    char* errmsg;
    std::lock_guard lk(db_mutex_);
    if (db_.get() == nullptr) {
      return std::unexpected(Error::DB_CONNECTION);
    }

    if (sqlite3_exec(db_.get(),
                     /* sql = */ args.sql.data(),
                     /* callback = */ args.callback,
                     /* callback_arg = */ args.callback_arg,
                     /* errmsg = */ &errmsg)) {
      log(errmsg);
      sqlite3_free(errmsg);
      return std::unexpected(Error::UNEXPECTED);
    }

    return sqlite3_changes(db_.get());
  }
};

Library::Library(std::unique_ptr<Impl> impl) : pimpl_(std::move(impl)) {}

Library::Library(Library&& that) noexcept { *this = std::move(that); }

auto Library::operator=(Library&& that) noexcept -> Library& {
  pimpl_ = std::exchange(that.pimpl_, std::make_unique<Impl>(nullptr));
  return *this;
}

Library::~Library() {}

auto make_library(std::string_view path)
    -> std::expected<Library, Library::Error> {
  sqlite3* db;
  if (sqlite3_open(path.data(), &db)) {
    log(sqlite3_errmsg(db));
    return std::unexpected(Library::Error::UNEXPECTED);
  }

  auto impl = std::make_unique<Library::Impl>(unique_sqlite3(db));
  return impl->execute(Library::Impl::ExecuteArgs{.sql = sql::INIT_DB_SQL})
      .transform([impl = std::move(impl)](int /* n affected rows */) mutable {
        return Library(std::move(impl));
      });
}

auto Library::insert(Book const& book) -> std::expected<UUID, Error> {
  Record rec{.uuid = UUID{},
             .isbn = book.isbn,
             .name = book.name,
             .author = book.author,
             .acquired = false};
  return pimpl_
      ->execute(Impl::ExecuteArgs{
          .sql = sql::make_insert_sql(rec),
      })
      .transform([uuid = rec.uuid](int /* n affected rows */) -> UUID {
        return uuid;
      });
}

auto Library::erase(UUID uuid) -> std::expected<void, Error> {
  return pimpl_
      ->execute(Impl::ExecuteArgs{
          .sql = sql::make_erase_sql(uuid),
      })
      .transform([](int /* n affected rows */) {});
}

auto Library::size() const -> std::expected<std::size_t, Error> {
  std::size_t count;
  return pimpl_
      ->execute(Impl::ExecuteArgs{
          .sql = sql::COUNT_SQL,
          .callback = parse_count,
          .callback_arg = &count,
      })
      .transform(
          [count](int /* n affected rows */) -> std::size_t { return count; });
}

auto Library::distinct() const -> std::expected<std::size_t, Error> {
  std::size_t count;
  return pimpl_
      ->execute(Impl::ExecuteArgs{
          .sql = sql::DISTINCT_SQL,
          .callback = parse_count,
          .callback_arg = &count,
      })
      .transform(
          [count](int /* n affected rows */) -> std::size_t { return count; });
}

auto Library::fetch_records_generic(std::string_view sql) const
    -> std::expected<std::vector<Record>, Error> {
  std::vector<Record> records;
  return pimpl_
      ->execute(Impl::ExecuteArgs{
          .sql = sql,
          .callback = parse_record,
          .callback_arg = &records,
      })
      .transform([records_ptr = &records](int) -> std::vector<Record> {
        return std::vector(std::move(*records_ptr));
      });
}

auto Library::records() const -> std::expected<std::vector<Record>, Error> {
  return fetch_records_generic(sql::RECORDS_SQL);
}

auto Library::name_like(std::string_view name_like)
    -> std::expected<std::vector<Record>, Error> {
  return fetch_records_generic(sql::make_name_like_sql(name_like));
}

auto Library::author_like(std::string_view author_like)
    -> std::expected<std::vector<Record>, Error> {
  return fetch_records_generic(sql::make_author_like_sql(author_like));
}

auto Library::acquisition_generic_sql(std::string_view sql) const
    -> std::expected<void, Error> {
  return pimpl_->execute(Impl::ExecuteArgs{.sql = sql})
      .and_then([](int changes) -> std::expected<void, Error> {
        if (changes == 1) {
          return {};
        }

        return std::unexpected(Error::INVALID_ARGUMENT);
      });
}

auto Library::acquire_book(UUID uuid) -> std::expected<void, Error> {
  return acquisition_generic_sql(
      sql::make_acquire_record_sql(UUIDString(uuid)));
}

auto Library::release_book(UUID uuid) -> std::expected<void, Error> {
  return acquisition_generic_sql(
      sql::make_release_record_sql(UUIDString(uuid)));
}

}  // namespace tbrekalo
