/*
 * File: random.hpp
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
 *	This header file defines the random_t class for generating random numbers using various distributions.
 */
#pragma once

#include <random>

namespace isw
{
    /**
     * @brief Provides random number generation using various distributions.
     * @details Uses Mersenne Twister engine for generating uniform and Gaussian random numbers.
     */
    class random_t
    {
    public:
        /**
         * @brief Constructor with specified seed.
         * @param[in] seed The seed for the random engine.
         */
        random_t( size_t seed );
        /**
         * @brief Default constructor using random device for seed.
         */
        random_t();

        /**
         * @brief Generates a uniform random integer in range [min, max].
         * @param[in] min Minimum value.
         * @param[in] max Maximum value.
         * @return Random integer.
         */
        int uniform_range( int min, int max );
        /**
         * @brief Generates a uniform random double in range [min, max).
         * @param[in] min Minimum value.
         * @param[in] max Maximum value.
         * @return Random double.
         */
        double uniform_range( double min, double max );
        /**
         * @brief Generates a Gaussian random sample.
         * @param[in] mean Mean of the distribution.
         * @param[in] stddev Standard deviation.
         * @return Random sample from normal distribution.
         */
        double gaussian_sample( double mean, double stddev );

        /**
         * @brief Gets reference to the random engine.
         * @return Reference to the Mersenne Twister engine.
         */
        std::mt19937_64 &get_engine();

    private:
        /** @brief Mersenne Twister random engine. */
        std::mt19937_64 _engine;
    };

} // namespace isw
