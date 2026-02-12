/*
 * File: lambda_parser.cpp
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
 *	This file implements the lambda_parser_t class methods for parsing input files.
 */
#include "io/lambda_parser.hpp"
using namespace isw;

lambda_parser_t::lambda_parser_t( const std::filesystem::path &path, const std::unordered_map< std::string, parser > &bindings ):
    input_parser_t( path ), _bindings( bindings ) {}

lambda_parser_t::lambda_parser_t( const std::filesystem::path &path, std::unordered_map< std::string, parser > &&bindings ):
    input_parser_t( path ), _bindings( std::move( bindings ) ) {}

void lambda_parser_t::parse() {
    std::string line;
    std::istringstream iss;
    std::string key;
    while(std::getline(get_stream(), line)) {
        iss = std::istringstream(line);
        iss >> key;
        auto it = _bindings.find(key);
        if (it != _bindings.end()) {
            auto& parser = it->second;
            parser(iss);
        }
        else {
            std::string err(" unknown input specified: ");
            err += key;
            throw std::runtime_error(err);
        }
    }
}