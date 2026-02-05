/*
 * File: input_parser.cpp
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
 *	This file implements the input_parser_t class methods for parsing input files.
 */
#include "io/input_parser.hpp"
#include <iostream>
#include <stdexcept>
// #include <sys/cdefs.h>
using namespace isw;


input_parser_t::input_parser_t( const std::filesystem::path &path )
{
    assert( std::filesystem::exists( path ) );
    assert( std::filesystem::is_regular_file( path ) );
    _stream = std::ifstream( path );

    if ( !_stream.is_open() )
    {
        std::string ss = "Failed to open file \"" + path.string() + "\"\n";
        throw std::runtime_error( ss.c_str() );
    }
}

void input_parser_t::reset_stream()
{
    _stream.clear();
    _stream.seekg( 0 );
}

std::ifstream &input_parser_t::get_stream() { return _stream; }

input_parser_t::~input_parser_t()
{
    if ( _stream.is_open() )
    {
        _stream.close();
    }
}

lambda_parser::lambda_parser( const std::filesystem::path &path, const std::unordered_map< std::string, Parser > &bindings ):
    input_parser_t( path ), _bindings( bindings ) {}

lambda_parser::lambda_parser( const std::filesystem::path &path, std::unordered_map< std::string, Parser > &&bindings ):
    input_parser_t( path ), _bindings( std::move( bindings ) ) {}

void lambda_parser::parse() {
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
        else 
            throw std::runtime_error(" unknown input specified ");
    }
}