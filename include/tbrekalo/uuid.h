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

  friend class UUIDString;

  unsigned char data_[SIZE];

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

class UUIDString {
  static inline constexpr int SIZE = UUID::TARGET_SIZE;
  char data_[SIZE];

  explicit UUIDString(std::string_view src);
  friend auto make_uuid_string(std::string_view) -> std::optional<UUIDString>;

 public:
  explicit UUIDString(UUID uuid) noexcept { uuid.serialize(data_); }

  auto size() const noexcept -> std::size_t { return UUID::TARGET_SIZE; }
  auto data(this auto&& self) -> decltype(auto) {
    return std::forward_like<decltype(self)>(self.data_);
  }

  explicit operator char const*() const {
    return static_cast<char const*>(data_);
  }
  explicit operator std::string_view() const { return std::string_view(data_); }
  explicit operator UUID() const;
};

auto make_uuid_string(std::string_view) -> std::optional<UUIDString>;

}  // namespace tbrekalo
