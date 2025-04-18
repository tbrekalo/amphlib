#pragma once

#include <memory>

#include "tbrekalo/book.h"

namespace tbrekalo {

class Storage {
 protected:
  struct Impl {
    virtual ~Impl();

    virtual auto upsert(Book const&) -> std::size_t = 0;
    virtual auto remove(ISBN) -> std::size_t = 0;

    virtual auto isbns() const -> std::vector<ISBN> = 0;
    virtual auto count(ISBN) const -> std::size_t = 0;
    virtual auto n_unique() const -> std::size_t = 0;
  };

  std::unique_ptr<Impl> pimpl_;
  Storage(std::unique_ptr<Impl> pimpl) : pimpl_(std::move(pimpl)) {}

 public:
  Storage() = delete;

  Storage(Storage const&) = delete;
  auto operator=(Storage const&) -> Storage& = delete;

  Storage(Storage&&) noexcept = delete;
  auto operator=(Storage&&) noexcept -> Storage& = delete;

  auto upsert(Book const& book) -> std::size_t { return pimpl_->upsert(book); }
  auto remove(ISBN isbn) -> std::size_t { return pimpl_->remove(isbn); }

  auto isbns() const -> std::vector<ISBN> { return pimpl_->isbns(); }
  auto count(ISBN isbn) const -> std::size_t { return pimpl_->count(isbn); };
  auto n_unique() const -> std::size_t { return pimpl_->n_unique(); };
};

class MemoryStorage : public Storage {
  class Impl;
  MemoryStorage();

 public:
  friend auto make_memory_storage() -> MemoryStorage;
};

auto make_memory_storage() -> MemoryStorage;

}  // namespace tbrekalo
