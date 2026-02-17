/*
 * File: array_parser.cpp
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
 *	This file implements the array_parser_t class methods for parsing input files.
 */
#include "io/array_parser.hpp"
#include <cassert>
#include <cstddef>
using namespace isw;

array_parser_t::array_parser_t( const std::filesystem::path &path, const std::vector< parser > &order ):
    input_parser_t( path ), _order( order ) {}

array_parser_t::array_parser_t( const std::filesystem::path &path, std::vector< parser > &&order ):
    input_parser_t( path ), _order( std::move( order ) ) {}

void array_parser_t::parse() {
    std::string line;
    std::istringstream iss;
    size_t idx = 0;
    assert(!_order.empty());
    while(std::getline(get_stream(), line)) {
        iss = std::istringstream(line);
        _order[std::min(idx, _order.size()-1)](iss);
        idx++;
    }
}

void array_parser_t::set_order(std::vector< parser > &&order) {
   _order = std::move(order);
}

void array_parser_t::set_order(const std::vector< parser > &order) {
    _order = order;
}