#pragma once

#include <memory>

#include "tbrekalo/book.h"

namespace tbrekalo {

class Storage {
 public:
  virtual ~Storage();

  virtual auto insert(Book const&) -> std::size_t = 0;
  virtual auto remove(ISBN) -> std::size_t = 0;

  virtual auto isbns() -> std::vector<ISBN> = 0;
  virtual auto count(ISBN) -> std::size_t = 0;
  virtual auto n_unique() -> std::size_t = 0;
};

class InMemoryStorage : public Storage {
  struct Impl;
  std::unique_ptr<Impl> pimpl_;

 public:
  InMemoryStorage();

  InMemoryStorage(InMemoryStorage const&) = delete;
  auto operator=(InMemoryStorage const&) -> InMemoryStorage& = delete;

  InMemoryStorage(InMemoryStorage&&) noexcept;
  auto operator=(InMemoryStorage&&) noexcept -> InMemoryStorage&;

  ~InMemoryStorage();

  auto insert(Book const&) -> std::size_t override;
  auto remove(ISBN) -> std::size_t override;

  auto isbns() -> std::vector<ISBN> override;
  auto count(ISBN) -> std::size_t override;
  auto n_unique() -> std::size_t override;
};

}  // namespace tbrekalo
