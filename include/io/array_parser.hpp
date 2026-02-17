/*
 * File: array_parser.hpp
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
 *	This header file defines the array_parser_t class for ordered parsing.
 */
#pragma once
#include <vector>
#include "io/input_parser.hpp"

namespace isw {

    class array_parser_t : public input_parser_t {
        public:
            array_parser_t( const std::filesystem::path &path, const std::vector< parser > &order );
            
            array_parser_t( const std::filesystem::path &path, std::vector< parser > &&order );
            
            void parse() override;
            
            void set_order(std::vector< parser > &&order);
            
            void set_order(const std::vector< parser > &order);

        private:
            std::vector< parser > _order;
    };

}