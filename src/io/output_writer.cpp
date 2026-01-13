/*
 * File: output_writer.cpp
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
 *	This file implements the output_writer_t class methods for writing output data.
 */
#include "io/output_writer.hpp"
#include "global.hpp"

using namespace isw;

output_writer_t::output_writer_t( const std::filesystem::path path ) :
    _stream(path)
{
    if ( !_stream.is_open() )
    {
        throw std::runtime_error( "Could not open log file" );
    }
}

output_writer_t::~output_writer_t() {
	if (_stream.is_open()) {
		_stream.close();
	}
}

void output_writer_t::write_line(const std::string& line) {
	_stream << line << "\n";
}

std::ofstream& output_writer_t::get_stream() {
	return _stream;
}

output_writer_t& isw::operator<<(output_writer_t& writer, std::ostream& (*manip)(std::ostream&)) {
    manip(writer.get_stream());
    return writer;
}

output_writer_t& isw::operator<<(output_writer_t& writer, std::ios_base& (*manip)(std::ios_base&)) {
    manip(writer.get_stream());
    return writer;
}
