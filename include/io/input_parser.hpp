/*
 * File: input_parser.hpp
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
 *	This header file defines the input_parser_t class for parsing input files and configuring the global state.
 */
#pragma once

#include <cassert>
#include <filesystem>
#include <fstream>

namespace isw
{

    /**
     * @brief Base class for parsing input files and configuring global state.
     * @details Provides stream access and global state management for input parsing.
     */
    class input_parser_t
    {
    public:
        /**
         * @brief Constructor.
         * @param[in] path Path to the input file.
         * @throws std::runtime_error If file cannot be opened.
         */
        input_parser_t( const std::filesystem::path &path = "parameters.txt" );
        ~input_parser_t();

        // // forward extraction to the underlying stream
        // template< typename T >
        // input_parser_t &operator>>( T &value )
        // {
        //     _stream >> value; // uses ifstream::operator>>
        //     return *this; //  i love c++
        // }

        /**
         * @brief Pure virtual parse method.
         * @details Must be implemented by subclasses to parse the input file.
         */
        virtual void parse() = 0;
        /**
         * @brief Gets the input stream.
         * @return Reference to the ifstream.
         */
        std::ifstream &get_stream();
        /**
         * @brief Resets the stream to the beginning.
         * @details Clears error flags and seeks to position 0.
         */
        void reset_stream();

    private:
        /** @brief Input file stream. */
        std::ifstream _stream;
    };
}