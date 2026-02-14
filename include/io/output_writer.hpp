/*
 * File: output_writer.hpp
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
 *	This header file defines the output_writer_t base class for writing simulation output to files.
 */
#pragma once

#include <filesystem>
#include <fstream>

namespace isw
{

    /**
     * @brief Base class for writing simulation output to files.
     * @details Provides file stream and global state access for output writing.
     */
    class output_writer_t
    {
    public:
        /**
         * @brief Constructor.
         * @param[in] path Path to the output file.
         * @param[in] global Shared pointer to global state.
         * @throws std::runtime_error If file cannot be opened.
         */
        output_writer_t( const std::filesystem::path path = "results.txt" );
        ~output_writer_t(); // Destructor needed to properly close the file stream.

        /**
         * @brief Places the formatted line into the buffer (made using make_line)
         * @details The buffer must be later flushed with save_output
         */
        void write_line(const std::string &line);
        std::ofstream& get_stream();

        // Friend template for arbitrary types T
        template<typename T>
        friend output_writer_t& operator<<(output_writer_t& writer, const T& value) {
            writer.get_stream() << value;
            return writer;
        }

        // Special overloads for stream manipulators (e.g., std::endl)
        friend output_writer_t& operator<<(output_writer_t& writer, std::ostream& (*manip)(std::ostream&));
        friend output_writer_t& operator<<(output_writer_t& writer, std::ios_base& (*manip)(std::ios_base&));
    private:
        /** @brief Output stream to the file. */
        std::ofstream _stream;
    };

    /**
     * @brief Output stream manipulator overload (e.g., std::endl).
     */
    output_writer_t& operator<<(output_writer_t& writer, std::ostream& (*manip)(std::ostream&));

    /**
     * @brief Output stream manipulator overload.
     */
    output_writer_t& operator<<(output_writer_t& writer, std::ios_base& (*manip)(std::ios_base&));
} // namespace isw
