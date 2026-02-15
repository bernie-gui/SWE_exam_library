/*
 * File: logger.cpp
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
 *	This file implements the logger_t class methods for logging data to files.
 */
#include "io/logger.hpp"
#include <iostream>
#include <stdexcept>
#include <string>

using namespace isw;

logger_t::logger_t( const std::filesystem::path &path ) : _block( false )
{
    _stream = std::ofstream( path );
    if ( !_stream.is_open() )
    {
        throw std::runtime_error( "Could not open log file" );
    }
}

std::shared_ptr< logger_t > logger_t::create( const std::filesystem::path &path )
{
    return std::shared_ptr< logger_t >( new logger_t( path ) );
};

std::shared_ptr< logger_t > logger_t::add_field( std::string_view field )
{
    if ( _block )
    {
        throw std::runtime_error( "Fields modified after the scheme has been defined" );
        return shared_from_this();
    }
    _fields.push_back( std::string( field ) );
    return shared_from_this();
}

std::shared_ptr< logger_t > logger_t::log_fields()
{
    if ( _block )
    {
        throw std::runtime_error( "Fields already logged" );
        return shared_from_this();
    }
    for ( const auto &field : _fields )
    {
        _stream << field << " ";
    }
    _block = true;
    _stream << std::endl;
    return shared_from_this();
}


std::shared_ptr< logger_t > logger_t::add_measurement( std::string_view value )
{
    _measurements.push_back( std::string( value ) );
    return shared_from_this();
}

std::shared_ptr< logger_t > logger_t::log_measurement()
{
    if ( _fields.size() != _measurements.size() )
    {
        throw std::runtime_error( "Log line does not fit the schema" );
        _measurements.clear();
        return shared_from_this();
    }
    for ( const auto &measurement : _measurements )
    {
        _stream << measurement << " ";
    }
    _stream << std::endl;
    _measurements.clear();
    return shared_from_this();
}
