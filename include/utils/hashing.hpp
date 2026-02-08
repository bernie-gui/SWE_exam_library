#pragma once

#include <functional>

template<>
struct std::hash<std::pair<size_t, size_t>> {
     size_t operator()(const std::pair<size_t, size_t>& p) const noexcept {
          size_t ret = 7;          
          ret = (ret + std::hash<size_t>()(p.first)) * 31;
          ret = (ret + std::hash<size_t>()(p.second)) * 31;
          return ret;
    }
};