#pragma once

#include <compare>
#include <expected>
#include <optional>
#include <span>
#include <string_view>
#include <utility>

namespace tbrekalo {

class UUID {
  static inline constexpr int SIZE = 16;
  static inline constexpr int TARGET_SIZE = 37;

  unsigned char data_[SIZE];

  friend class UUIDString;
  friend struct std::hash<UUID>;

 public:
  using SourceSpan = std::span<unsigned char, SIZE>;
  using TargetSpan = std::span<char, TARGET_SIZE>;

  UUID();
  explicit UUID(SourceSpan source);
  auto serialize(TargetSpan target) const -> void;

  auto size() const noexcept -> std::size_t { return SIZE; }
  auto data(this auto&& self) -> decltype(auto) {
    return std::forward_like<decltype(self)>(self.data_);
  }

  friend inline auto operator<=>(UUID const&, UUID const&) noexcept
      -> std::strong_ordering = default;
};

}  // namespace tbrekalo

namespace std {

template <>
struct hash<tbrekalo::UUID> {
  // djb2
  auto operator()(tbrekalo::UUID uuid) const noexcept -> std::size_t {
    std::size_t hash = 5381;
    for (int i = 0; i < tbrekalo::UUID::SIZE; ++i) {
      hash = ((hash << 5) + hash) + uuid.data_[i];
    }
    return hash;
  }
};

}  // namespace std

namespace tbrekalo {

class UUIDString {
  static inline constexpr int DATA_SIZE = 37;
  static inline constexpr int STRING_LENGTH = 36;
  char data_[37];

  explicit UUIDString(std::string_view src);

  friend struct std::hash<UUIDString>;
  friend auto make_uuid_string(std::string_view) -> std::optional<UUIDString>;

 public:
  explicit UUIDString(UUID uuid) noexcept { uuid.serialize(data_); }

  auto size() const noexcept -> std::size_t { return STRING_LENGTH; }
  auto data(this auto&& self) -> decltype(auto) {
    return std::forward_like<decltype(self)>(self.data_);
  }

  explicit operator char const*() const {
    return static_cast<char const*>(data_);
  }
  explicit operator std::string_view() const { return std::string_view(data_); }
  explicit operator UUID() const;

  friend inline auto operator<=>(UUIDString const&, UUIDString const&) noexcept
      -> std::strong_ordering = default;
};

auto make_uuid_string(std::string_view) -> std::optional<UUIDString>;

}  // namespace tbrekalo

namespace std {

template <>
struct hash<tbrekalo::UUIDString> {
  // djb2
  auto operator()(tbrekalo::UUIDString uuid) const noexcept -> std::size_t {
    std::size_t hash = 5381;
    for (int i = 0; i < tbrekalo::UUIDString::STRING_LENGTH; ++i) {
      hash = ((hash << 5) + hash) + uuid.data_[i];
    }
    return hash;
  }
};

}  // namespace std
