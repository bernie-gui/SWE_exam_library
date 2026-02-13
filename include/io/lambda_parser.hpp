/*
 * File: lambda_parser.hpp
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
 *	This header file defines the lambda_parser_t class for fast-implemantable parsing.
 */
#pragma once

#include <unordered_map>
#include <functional>
#include "io/input_parser.hpp"

namespace isw {

    using parser = std::function<void(std::istringstream&)>;

    /**
     * @brief Input parser implementation using lambdas.
     * @details Operates on key - value maps of types <std::string, Parser>.
     */    
    class lambda_parser_t : public input_parser_t {
        public:
            /**
            * @brief Constructor.
            * @param[in] path Path to the input file.
            * @param[in] bindings Map of line starters to corresponding parsing lambdas (lvalue).
            * @throws std::runtime_error If file cannot be opened.
            */
            lambda_parser_t( const std::filesystem::path &path, const std::unordered_map< std::string, parser > &bindings );
            /**
            * @brief Constructor.
            * @param[in] path Path to the input file.
            * @param[in] bindings Map of line starters to corresponding parsing lambdas (rvalue).
            * @throws std::runtime_error If file cannot be opened.
            */
            lambda_parser_t( const std::filesystem::path &path, std::unordered_map< std::string, parser > &&bindings );
            /**
            * @brief Overridden parse method.
            * @details Calls the respective lambda parser for each input line.
            */
            void parse() override;
            //TODO: documentation
            void set_bindings(std::unordered_map< std::string, parser > &&bindings);
            void set_bindings(const std::unordered_map< std::string, parser > &bindings);
        private:
            /** @brief Lambda bindings map. */    
            std::unordered_map< std::string, parser > _bindings;
    };
}