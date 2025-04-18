#pragma once

#include <compare>
#include <expected>
#include <string_view>

namespace tbrekalo {

class ISBN {
  static inline constexpr int BUFFER_SIZE = 14;
  static inline constexpr char SENTINEL = '\0';
  friend struct std::hash<ISBN>;

  unsigned char buffer_[BUFFER_SIZE];

 public:
  enum class Error : char { INVALID_LENGTH, INVALID_CHAR };

  friend constexpr auto make_isbn(std::string_view) noexcept
      -> std::expected<ISBN, Error>;
  friend inline constexpr auto operator<=>(ISBN const&, ISBN const&) noexcept
      -> std::strong_ordering = default;

  explicit operator std::string_view() const noexcept {
    return std::string_view(reinterpret_cast<char const*>(&buffer_));
  }
};

inline constexpr auto make_isbn(std::string_view src) noexcept
    -> std::expected<ISBN, ISBN::Error> {
  if (src.length() != 10 && src.length() != 13) {
    return std::unexpected(ISBN::Error::INVALID_LENGTH);
  }

  ISBN dst;
  std::size_t i = 0;
  // TODO(tbrekalo): implement ISBN check sum.
  for (; i < src.length(); ++i) {
    dst.buffer_[i] = src[i];
    if (src[i] < '0' || src[i] > '9') {
      return std::unexpected(ISBN::Error::INVALID_CHAR);
    }
  }

  dst.buffer_[i] = ISBN::SENTINEL;
  return dst;
}

}  // namespace tbrekalo

namespace std {

template <>
struct hash<tbrekalo::ISBN> {
  constexpr auto operator()(tbrekalo::ISBN isbn) const noexcept -> std::size_t {
    std::size_t hash = 5381;
    for (int i = 0; i < tbrekalo::ISBN::BUFFER_SIZE; ++i) {
      hash = ((hash << 5) + hash) + isbn.buffer_[i];
    }

    return hash;
  }
};

}  // namespace std
