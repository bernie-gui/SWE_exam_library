/*
 * File: common.hpp
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
 *	This header file defines common types and utilities used throughout the simulation library.
 */
#pragma once

#include <cstdint>
#include <string>
/*
 * TYPES DEFINITIONs
 */
/** @brief 8-bit unsigned integer type. */
typedef uint8_t u8_t;
/** @brief 16-bit unsigned integer type. */
typedef uint16_t u16_t;
/** @brief 32-bit unsigned integer type. */
typedef uint32_t u32_t;
/** @brief 64-bit unsigned integer type. */
typedef uint64_t u64_t;

namespace isw
{
    /** @brief Type for world keys, represented as strings. */
    using world_key_t = std::string;
} // namespace isw
