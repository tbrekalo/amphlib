# Amphlib

![ci-status](https://github.com/tbrekalo/amphlib/actions/workflows/ci.yml/badge.svg)

Amphlib is a demo project implementing a book library management system utilizing functional features of modern C++. Usage examples can be found in `src/test.cc`. `misc/google-books.ipynb` is a Jupyter notebook for fetching sample data from the Google Books API.

## Dependencies

- Linux distro or macOS (recommended)

### C++

- clang >= 20
- cmake >= 3.25
- doctest >= 2.4.11
- git >= 2.39
- GNU make >= 4.4.1 (optional)
- libuuid >= 2.41
- sqlite3 >= 3.49


### Python
- [uv](https://docs.astral.sh/uv/)


### Building and running tests locally

```bash
uv sync
source .venv/bin/activate

make build-debug
./build-debug/bin/test
```

### Run tests using [act](https://github.com/nektos/act)

```bash
act
```

## Interface example

```cpp
class Library {
  class Impl;

  enum class Error : char { DB_CONNECTION, INVALID_ARGUMENT, UNEXPECTED };

  struct Record {
    UUID uuid;
    ISBN isbn;
    std::string name;
    std::string author;
    bool acquired;
  };

  mutable std::unique_ptr<Impl> pimpl_;
  explicit Library(std::unique_ptr<Impl>);

  auto fetch_records_generic(std::string_view sql) const
      -> std::expected<std::vector<Record>, Error>;
  auto acquisition_generic_sql(std::string_view sql) const
      -> std::expected<void, Error>;

 public:
  using Error = Error;
  using Record = Record;

  Library(Library const&) = delete;
  auto operator=(Library const&) -> Library& = delete;

  Library(Library&&) noexcept;
  auto operator=(Library&&) noexcept -> Library&;

  ~Library();

  auto insert(Book const&) -> std::expected<UUID, Error>;
  auto erase(UUID) -> std::expected<void, Error>;

  auto size() const -> std::expected<std::size_t, Error>;
  auto distinct() const -> std::expected<std::size_t, Error>;

  auto records() const -> std::expected<std::vector<Record>, Error>;

  auto name_like(std::string_view) -> std::expected<std::vector<Record>, Error>;
  auto author_like(std::string_view)
      -> std::expected<std::vector<Record>, Error>;

  auto acquire_book(UUID) -> std::expected<void, Error>;
  auto release_book(UUID) -> std::expected<void, Error>;

  friend auto make_library(std::string_view path)
      -> std::expected<Library, Library::Error>;
};
```