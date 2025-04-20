#pragma once

#include <expected>
#include <memory>
#include <vector>

#include "tbrekalo/book.h"
#include "tbrekalo/uuid.h"

namespace tbrekalo {

class Library {
  class Impl;

  enum class Error : char { DB_CONNECTION, INVALID_ARGUMENT, UNEXPECTED };

  struct Record {
    UUID uuid;
    ISBN isbn;
    std::string name;
    std::string author;
    bool acquired;
  };

  mutable std::unique_ptr<Impl> pimpl_;
  explicit Library(std::unique_ptr<Impl>);

  auto fetch_records_generic(std::string_view sql) const
      -> std::expected<std::vector<Record>, Error>;
  auto acquisition_generic_sql(std::string_view sql) const
      -> std::expected<void, Error>;

 public:
  using Error = Error;
  using Record = Record;

  Library(Library const&) = delete;
  auto operator=(Library const&) -> Library& = delete;

  Library(Library&&) noexcept;
  auto operator=(Library&&) noexcept -> Library&;

  ~Library();

  auto insert(Book const&) -> std::expected<UUID, Error>;
  auto erase(UUID) -> std::expected<void, Error>;

  auto size() const -> std::expected<std::size_t, Error>;
  auto distinct() const -> std::expected<std::size_t, Error>;

  auto records() const -> std::expected<std::vector<Record>, Error>;

  auto name_like(std::string_view) -> std::expected<std::vector<Record>, Error>;
  auto author_like(std::string_view)
      -> std::expected<std::vector<Record>, Error>;

  auto acquire_book(UUID) -> std::expected<void, Error>;
  auto release_book(UUID) -> std::expected<void, Error>;

  friend auto make_library(std::string_view path)
      -> std::expected<Library, Library::Error>;
};

auto make_library(std::string_view path)
    -> std::expected<Library, Library::Error>;

}  // namespace tbrekalo
