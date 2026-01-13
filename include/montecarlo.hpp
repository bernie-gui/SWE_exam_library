/*
 * File: montecarlo.hpp
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
 *	This header file defines the montecarlo_t class for performing Monte Carlo simulations.
 */
#pragma once

#include <memory>
#include "simulator.hpp"
namespace isw
{
    /**
     * @brief Performs Monte Carlo simulations by running multiple simulation instances and averaging results.
     */
    class montecarlo_t
    {
    public:
        /**
         * @brief Runs the Monte Carlo simulation.
         * @details Initializes the average to 0, then runs the simulator for the budgeted number of times,
         * updating the running average of the Monte Carlo current values.
         */
        void run();
        /**
         * @brief Gets the simulator instance.
         * @return Shared pointer to the simulator.
         */
        std::shared_ptr< simulator_t > get_simulator() const;
        /**
         * @brief Factory method to create a montecarlo_t instance.
         * @param[in] sim Shared pointer to the simulator.
         * @return Shared pointer to the created montecarlo_t.
         */
        static std::shared_ptr< montecarlo_t > create( const std::shared_ptr< simulator_t > sim );

        template< typename SIM >
        /**
         * @brief Template factory method to create a montecarlo_t with a specific simulator type.
         * @tparam SIM The simulator type to create.
         * @param[in] sys Shared pointer to the system.
         * @return Shared pointer to the created montecarlo_t.
         */
        static std::shared_ptr< montecarlo_t > create( const std::shared_ptr< system_t > sys )
        {
            return std::shared_ptr< montecarlo_t >( new montecarlo_t( std::make_shared< SIM >( sys ) ) );
        }

    private:
        /** @brief Private constructor.
         * @param[in] sim Shared pointer to the simulator.
         */
        montecarlo_t( std::shared_ptr< simulator_t > sim );
        void _init();                        /**< @brief Initialization method (currently unused). */
        std::shared_ptr< simulator_t > _sim; /**< @brief The simulator instance. */
    };
} // namespace isw
