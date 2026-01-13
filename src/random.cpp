/*
 * File: random.cpp
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
 *	This file implements the random_t class methods for random number generation.
 */
#include "random.hpp"
#include <random>
using namespace isw;

random_t::random_t( size_t seed ) : _engine( seed ) {}

random_t::random_t() : random_t( std::random_device{}() ) {};

std::mt19937_64 &random_t::get_engine() { return _engine; }

int random_t::uniform_range( int min, int max )
{
    std::uniform_int_distribution< int > dist( min, max );
    return dist( _engine );
}

double random_t::uniform_range( double min, double max )
{
    std::uniform_real_distribution< double > dist( min, max );
    return dist( _engine );
}

double random_t::gaussian_sample( double mean, double stddev )
{
    std::normal_distribution<> dist( mean, stddev );
    return dist( _engine );
}
