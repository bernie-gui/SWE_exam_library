#pragma once

#include <cstddef>
#include <functional>
#include <vector>

// TODO: documentation (?)
template<typename T, typename R>
struct std::hash<std::pair<T, R>> {
     size_t operator()(const std::pair<T, R>& p) const noexcept {
          size_t ret = 7;          
          ret = (ret + std::hash<T>()(p.first)) * 31;
          ret = (ret + std::hash<R>()(p.second)) * 31;
          return ret;
    }
};

template<typename T>
struct std::hash<std::vector<T>> {
     size_t operator()(const std::vector<T>& p) const noexcept {
          size_t ret = 7;     
          for (const auto &e : p)     
               ret = (ret + std::hash<T>()(e)) * 31;
          return ret;
    }
};