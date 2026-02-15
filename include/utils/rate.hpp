/*
 * File: rate.hpp
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
 *	This header file defines the rate_meas_t class for rate measurements in the simulation system.
 */
#pragma once

namespace isw::utils {

    /**
     * @brief Utility class for computing running rate measurements.
     * @details Maintains an incrementally updated rate using the formula:
     *   rate = old_rate * (old_denom / new_denom) + amount / new_denom.
     *   Useful for tracking throughput, arrival rates, or other per-unit metrics in simulations.
     */
    class rate_meas_t {
        public:
            /**
             * @brief Default constructor.
             * @details Initializes rate and last denominator to 0.
             */
            rate_meas_t();

            /**
             * @brief Updates the rate with a new observation.
             * @param[in] amount The increment to add to the numerator.
             * @param[in] denom The new denominator value (e.g. current simulation time).
             * @throws std::runtime_error If denom is zero.
             */
            void update(double amount, double denom);

            /**
             * @brief Updates the rate without adding new observations.
             * @param[in] denom The new denominator value (e.g. current simulation time).
             * @details Equivalent to calling update(0, denom).
             */
            void update(double denom);

            /**
             * @brief Returns the current rate value.
             * @return The computed rate.
             */
            double get_rate();

            /**
             * @brief Resets the rate measurement to its initial state.
             * @details Sets both rate and last denominator to 0.
             */
            void init();

        private:
            /** @brief Current rate value. */
            double _rate;
            /** @brief Last denominator used for the update formula. */
            double _last_denom;
    };
}