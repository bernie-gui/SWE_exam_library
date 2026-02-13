/*
 * File: functions.hpp
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
 *	This header file defines vehicle-related utilities.
 */
#pragma once

#include <cstddef>
#include "utils/vehicles/vehicle.hpp"

//TODO: documentation
namespace isw::uv {
    size_t count_collisions(const std::vector<std::shared_ptr<vehicle_t>> &vehicles, 
        double coll_radius, size_t dimensions);

    double euclidean_distance(std::shared_ptr<vehicle_t> v1, std::shared_ptr<vehicle_t> v2, size_t dimensions);
}
