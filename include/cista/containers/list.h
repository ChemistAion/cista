#pragma once

#include <list>

namespace cista {

template <typename T>
using list = std::list<T>;

namespace raw {
using cista::list;
}  // namespace raw

namespace offset {
using cista::list;
}  // namespace offset

}  // namespace cista
