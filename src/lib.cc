#include "tbrekalo/lib.h"

namespace tbrekalo {

struct Library::Impl {};

Library::Library(Library&& that) noexcept : pimpl_(std::move(that.pimpl_)) {}

auto Library::operator=(Library&& that) noexcept -> Library& {
  pimpl_ = std::move(that.pimpl_);
  return *this;
}

Library::~Library() {}

}  // namespace tbrekalo
