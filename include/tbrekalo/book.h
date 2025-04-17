#pragma once

#include <string>
#include <vector>

#include "tbrekalo/isbn.h"

namespace tbrekalo {

struct Book {
  ISBN isbn;
  std::string name;
  std::vector<std::string> authors;
};

}  // namespace tbrekalo
