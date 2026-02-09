/*
 * File: hashing.hpp
 * Copyright (c) 2025 bernie_gui, uniquadev, SepeFr.
 *
 * This file is part of SWE_exam_library
 *
 * SWE_exam_library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SWE_exam_library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * University: Sapienza University of Rome
 * Instructor: Enrico Tronci
 * Academic Year: 2025-2026
 *
 * Description:
 *	This header file defines hashing methods commonly used throughout the library.
 */
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