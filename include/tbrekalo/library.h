#pragma once

#include <memory>

#include "tbrekalo/book.h"
#include "tbrekalo/uuid.h"

namespace tbrekalo {

class Library {
  struct Impl;
  std::unique_ptr<Impl> pimpl_;

  Library(std::string_view path);
  friend auto std::make_unique<Impl>() -> std::unique_ptr<Impl>;
  friend auto make_library(std::string_view path) -> std::unique_ptr<Library>;

 public:
  struct LikeResult {
    UUID uuid;
    Book book;
  };

  Library(Library const&) = delete;
  auto operator=(Library const&) -> Library& = delete;

  Library(Library&&) noexcept = delete;
  auto operator=(Library&&) noexcept -> Library& = delete;

  ~Library();

  auto insert(Book const&) -> UUID;
  auto erase(UUID) -> void;

  auto author_like(std::string_view) -> std::vector<LikeResult>;
  auto title_like(std::string_view) -> std::vector<LikeResult>;
};

auto make_library(std::string_view path) -> std::unique_ptr<Library>;

}  // namespace tbrekalo
