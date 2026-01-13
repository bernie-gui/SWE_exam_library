/*
 * File: simulator.hpp
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
 *	This header file defines the simulator_t base class for running simulations with termination conditions.
 */
#pragma once

#include <cassert>
#include <memory>
#include "io/logger.hpp"
#include "system.hpp"

namespace isw
{
    /**
     * @brief Base class for running simulations with termination conditions.
     * @details Manages the simulation loop, initializing the system and stepping until termination.
     */
    class simulator_t
    {
    public:
        /**
         * @brief Constructor.
         * @param[in] sys Shared pointer to the system to simulate.
         */
        simulator_t( const std::shared_ptr< system_t > sys );
        /**
         * @brief Runs the simulation.
         * @details Initializes the system, then steps until termination condition is met, then calls on_terminate.
         */
        virtual void run();
        /**
         * @brief Pure virtual termination condition.
         * @return True if simulation should terminate.
         * @details Must be overridden by subclasses to define when to stop the simulation.
         */
        virtual bool should_terminate();
        /**
         * @brief Called after simulation terminates.
         * @details Virtual method for cleanup or final actions, default does nothing.
         */
        virtual void on_terminate();

        /**
         * @brief Gets the system.
         * @return Shared pointer to the system.
         */
        std::shared_ptr< system_t > get_system();

        template< typename T = global_t >
        std::shared_ptr< T > get_global()
        {
            auto casted = std::dynamic_pointer_cast< T >( get_system()->get_global() );
            assert( casted );
            return casted;
        }

    private:
        /** @brief Logger instance (currently unused). */
        std::unique_ptr< logger_t > _logger;
        /** @brief The system being simulated. */
        std::shared_ptr< system_t > _system;
    };
} // namespace isw
