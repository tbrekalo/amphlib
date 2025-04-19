#pragma once

#include <expected>
#include <memory>
#include <vector>

#include "tbrekalo/book.h"
#include "tbrekalo/uuid.h"

namespace tbrekalo {

class Library {
  class Impl;
  mutable std::shared_ptr<Impl> pimpl_;

  explicit Library(std::shared_ptr<Impl>);

 public:
  enum class Error : char { INTERNAL, INVALID_ARGUMENT };

  struct Record {
    UUID uuid;
    ISBN isbn;
    std::string name;
    std::string author;
    bool available;
  };

  auto insert(Book const&) -> std::expected<UUID, Error>;
  auto erase(UUID) -> std::expected<void, Error>;

  auto size() const -> std::expected<std::size_t, Error>;
  auto distinct() const -> std::expected<std::size_t, Error>;

  auto name_like(std::string_view) -> std::expected<std::vector<Record>, Error>;
  auto author_like(std::string_view)
      -> std::expected<std::vector<Record>, Error>;

  friend auto make_library(std::string_view path)
      -> std::expected<Library, Library::Error>;
};

auto make_library(std::string_view path)
    -> std::expected<Library, Library::Error>;

}  // namespace tbrekalo
