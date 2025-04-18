#include "tbrekalo/storage.h"

#include <ranges>
#include <unordered_map>
#include <vector>

namespace tbrekalo {

Storage::Impl::~Impl() {}

class MemoryStorage::Impl final : public Storage::Impl {
  std::unordered_map<ISBN, Book> books_;
  std::unordered_map<ISBN, std::size_t> counts_;

 public:
  ~Impl() {}

  auto upsert(Book const& book) -> std::size_t override {
    books_[book.isbn] = book;
    return ++counts_[book.isbn];
  }

  auto remove(ISBN isbn) -> std::size_t override {
    auto it = counts_.find(isbn);
    if (it == counts_.end()) {
      return 0;
    }

    if (it->second == 1) {
      books_.erase(isbn);
      counts_.erase(it);
      return 0;
    }

    return --it->second;
  }

  auto isbns() const -> std::vector<ISBN> override {
    std::vector<ISBN> dst(books_.size());
    std::ranges::copy(
        std::views::transform(books_,
                              [](std::pair<ISBN, Book> const& pair) -> ISBN {
                                return pair.first;
                              }),
        dst.begin());
    return dst;
  }

  auto count(ISBN isbn) const -> std::size_t override {
    return counts_.at(isbn);
  }

  auto n_unique() const -> std::size_t override { return books_.size(); }
};

MemoryStorage::MemoryStorage() : Storage(std::make_unique<Impl>()) {}
auto make_memory_storage() -> MemoryStorage { return MemoryStorage(); }

}  // namespace tbrekalo
