#pragma once

#include <memory>

#include "tbrekalo/book.h"

namespace tbrekalo {

class Storage {
 public:
  virtual auto store_book(Book const&) -> void = 0;
  virtual auto list_isbns() -> std::vector<ISBN> = 0;
};

class InMemoryStorage : public Storage {
  struct Impl;
  std::unique_ptr<Impl> pimpl_;

 public:
  auto store_book(Book const&) -> void override;
  auto list_isbns() -> std::vector<ISBN> override;
};

}  // namespace tbrekalo
