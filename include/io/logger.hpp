/*
 * File: logger.hpp
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
 *	This header file defines the logger_t class for logging simulation data to CSV files.
 */
#pragma once

#include <filesystem>
#include <fstream>
#include <memory>
#include <string_view>
#include <vector>

namespace isw
{
    /**
     * @brief Logs simulation data to CSV files.
     * @details Manages field schema and measurement logging with chaining support.
     */
    class logger_t : public std::enable_shared_from_this< logger_t >
    {
    public:
        /**
         * @brief Factory method to create a logger.
         * @param[in] path Path to the log file, defaults to "logfile.csv".
         * @return Shared pointer to created logger.
         */
        static std::shared_ptr< logger_t > create( const std::filesystem::path &path = "logfile.csv" );
        /**
         * @brief Adds a field to the schema.
         * @param[in] field Field name.
         * @return Shared pointer to this logger for chaining.
         * @throws std::runtime_error If fields are modified after logging schema.
         */
        std::shared_ptr< logger_t > add_field( std::string_view field );
        /**
         * @brief Logs the field schema to the file.
         * @return Shared pointer to this logger for chaining.
         * @throws std::runtime_error If fields already logged.
         */
        std::shared_ptr< logger_t > log_fields();
        /**
         * @brief Adds a measurement value.
         * @param[in] value Measurement value.
         * @return Shared pointer to this logger for chaining.
         */
        std::shared_ptr< logger_t > add_measurement( std::string_view value );
        /**
         * @brief Logs the current measurements.
         * @return Shared pointer to this logger for chaining.
         * @throws std::runtime_error If measurement count doesn't match field count.
         */
        std::shared_ptr< logger_t > log_measurement();

    private:
        /** @brief Private constructor.
         * @param[in] path Path to the log file.
         * @throws std::runtime_error If file cannot be opened.
         */
        logger_t( const std::filesystem::path &path );
        /** @brief Flag to block field modifications after logging schema. */
        bool _block; /* = false */
                     /** @brief Output stream to the log file. */
        std::ofstream _stream;
        /** @brief List of field names. */
        std::vector< std::string > _fields;
        /** @brief List of current measurements. */
        std::vector< std::string > _measurements;
    };
} // namespace isw
