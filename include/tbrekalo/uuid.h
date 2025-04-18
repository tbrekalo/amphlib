#pragma once

#include <compare>
#include <span>

namespace tbrekalo {

class UUID {
  static inline constexpr int SIZE = 16;
  static inline constexpr int TARGET_SIZE = 37;

  unsigned char data_[SIZE];

 public:
  using SourceSpan = std::span<unsigned char, SIZE>;
  using TargetSpan = std::span<char, TARGET_SIZE>;

  UUID();
  explicit UUID(SourceSpan source);
  auto serialize(TargetSpan target) const -> void;

  friend inline auto operator<=>(UUID const&, UUID const&) noexcept
      -> std::strong_ordering = default;
};

}  // namespace tbrekalo
