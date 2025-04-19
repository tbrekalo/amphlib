#pragma once

#include <string>

#include "tbrekalo/isbn.h"

namespace tbrekalo {

struct Book {
  ISBN isbn;
  std::string name;
  std::string author;
};

}  // namespace tbrekalo
