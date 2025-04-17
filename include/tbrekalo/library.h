#pragma once

#include "tbrekalo/storage.h"

namespace tbrekalo {

class Library {
  struct Impl;
  std::unique_ptr<Impl> pimpl_;

 public:
  Library(std::unique_ptr<Storage>);

  Library(Library const&) = delete;
  auto operator=(Library const&) -> Library& = delete;

  Library(Library&&) noexcept;
  auto operator=(Library&&) noexcept -> Library&;

  ~Library();

  auto add(Book) -> bool;
  auto list(Book) -> std::vector<ISBN>;
};

}  // namespace tbrekalo
