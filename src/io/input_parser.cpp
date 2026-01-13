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
// #include <sys/cdefs.h>
using namespace isw;


input_parser_t::input_parser_t( std::shared_ptr< global_t > global, const std::filesystem::path &path ) :
    _global( global )
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

// very important tutorial right here
// this is for format
// TYPE VALUE
// e.g. H 100 \n W 120 \n C 130.01
// or: ... \n COST 130.01
// void how_to_parse( std::ifstream file )
// {

//     double very_important_variable;
//     double mututo_di_casa;
//     int horizon;

//     std::string line;
//     while ( std::getline( file, line ) )
//     {
//         std::istringstream iss( line );
//         char tag;

//         // malformatted
//         if ( !( iss >> tag ) )
//             continue;

//         switch ( tag )
//         {
//             case 'C':
//                 iss >> mututo_di_casa;
//                 break;
//             case 'H':
//                 iss >> horizon;
//                 break;
//             case 'V':
//                 iss >> very_important_variable;
//                 break;
//             default:
//                 break;
//         }
//     }
// }

// parsing files like
// A 1 2 3 4
// if (sscanf(line, "A %d %d %lf %lf", &i, &j, &prob, &cost) >= 4) {
// void how_to_parse2( std::ifstream file )
// {

//     double very_important_variable;
//     double mututo_di_casa, prob, cost;
//     int horizon = 0, i = 0, j = 0;

//     std::string line;
//     while ( std::getline( file, line ) )
//     {
//         std::istringstream iss( line );
//         char tag;

//         // mal formatted
//         if ( !( iss >> tag ) )
//         {
//             continue;
//         }

//         if ( tag == 'A' )
//         {
//             // cpp wizardy
//             if ( iss >> i >> j >> prob >> cost )
//             {
//                 very_important_variable = i;
//                 mututo_di_casa = j;
//                 horizon = cost;
//                 // assign variables
//             }
//         }
//     }
//     std:: cout << mututo_di_casa << very_important_variable << horizon << std::endl;
// }
