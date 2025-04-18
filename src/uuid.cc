#include "tbrekalo/uuid.h"

#include <uuid/uuid.h>

#include <cstring>

namespace tbrekalo {

UUID::UUID() { uuid_generate(data_); }

UUID::UUID(std::span<unsigned char, SIZE> source) {
  std::memcpy(&data_, source.data(), SIZE);
}

auto UUID::serialize(std::span<char, UUID::TARGET_SIZE> target) const -> void {
  uuid_unparse(data_, target.data());
}

}  // namespace tbrekalo
