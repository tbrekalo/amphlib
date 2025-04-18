#include "tbrekalo/storage.h"

#include <ranges>
#include <unordered_map>
#include <utility>
#include <vector>

#include "tbrekalo/book.h"

namespace tbrekalo {

Storage::~Storage() {}

struct InMemoryStorage::Impl {
  std::unordered_map<ISBN, Book> books;
  std::unordered_map<ISBN, std::size_t> counts;
};

InMemoryStorage::InMemoryStorage()
    : pimpl_(std::make_unique<InMemoryStorage::Impl>()) {}

InMemoryStorage::InMemoryStorage(InMemoryStorage&& that) noexcept
    : pimpl_(std::exchange(that.pimpl_,
                           std::make_unique<InMemoryStorage::Impl>())) {}

auto InMemoryStorage::operator=(InMemoryStorage&& that) noexcept
    -> InMemoryStorage& {
  pimpl_ =
      std::exchange(that.pimpl_, std::make_unique<InMemoryStorage::Impl>());
  return *this;
}

InMemoryStorage::~InMemoryStorage() {}

auto InMemoryStorage::insert(Book const& book) -> std::size_t {
  pimpl_->books[book.isbn] = book;
  return ++pimpl_->counts[book.isbn];
}

auto InMemoryStorage::remove(ISBN isbn) -> std::size_t {
  auto it = pimpl_->counts.find(isbn);
  if (--it->second == 0) {
    pimpl_->counts.erase(it);
    pimpl_->books.erase(isbn);

    return 0;
  }

  return it->second;
}

auto InMemoryStorage::isbns() -> std::vector<ISBN> {
  std::vector<ISBN> dst(pimpl_->books.size());
  std::ranges::copy(
      std::views::transform(
          pimpl_->books,
          [](std::pair<ISBN, Book> const& pair) -> ISBN { return pair.first; }),
      dst.begin());

  return dst;
}

auto InMemoryStorage::count(ISBN isbn) -> std::size_t {
  return pimpl_->counts[isbn];
}

auto InMemoryStorage::n_unique() -> std::size_t {
  return pimpl_->books.size();
};

}  // namespace tbrekalo
