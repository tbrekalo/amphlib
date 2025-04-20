#include "tbrekalo/uuid.h"

#include <uuid/uuid.h>

#include <cassert>
#include <cstring>

namespace tbrekalo {

UUID::UUID() { uuid_generate(data_); }

UUID::UUID(std::span<unsigned char, SIZE> source) {
  std::memcpy(&data_, source.data(), SIZE);
}

auto UUID::serialize(std::span<char, UUID::TARGET_SIZE> target) const -> void {
  uuid_unparse(data_, target.data());
}

UUIDString::UUIDString(std::string_view source) {
  assert(source.size() == STRING_LENGTH);
  std::memcpy(data_, source.data(), UUID::TARGET_SIZE);
}

auto make_uuid_string(std::string_view source) -> std::optional<UUIDString> {
  // TODO(tbrekalo): do better checks on UUID?
  if (source.size() != UUIDString::STRING_LENGTH) {
    return std::nullopt;
  }

  return UUIDString(source);
}

UUIDString::operator UUID() const {
  uuid_t uuid;
  uuid_parse(data_, uuid);
  return UUID(uuid);
}

}  // namespace tbrekalo
