/*
 * File: vehicle.cpp
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
 *	This file implements vehicle utility functions.
 */
#include "utils/vehicles/functions.hpp"

size_t isw::uv::count_collisions( const std::vector< std::shared_ptr< vehicle_t > > &vehicles, double coll_radius )
{
    size_t id1, id2, collision_count = 0;
    double dist = 0;
    for ( auto v1 : vehicles )
        for ( auto v2 : vehicles )
        {
            id1 = v1->get_relative_id().value();
            id2 = v2->get_relative_id().value();
            if ( id1 >= id2 )
                continue;
            dist = euclidean_distance( v1, v2 );
            if ( dist > coll_radius )
                continue;
            collision_count++;
        }
    return collision_count;
}

double isw::uv::euclidean_distance( std::shared_ptr< vehicle_t > v1, std::shared_ptr< vehicle_t > v2 )
{
    double temp, dist = 0;
    for ( size_t i = 0; i < v1->pos.size(); i++ )
    {
        temp = v1->pos[i] - v2->pos[i];
        dist += temp * temp;
    }
    dist = std::sqrt( dist );
    return dist;
}
