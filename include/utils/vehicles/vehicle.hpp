/*
 * File: vehicle.hpp
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
 *	This header file defines the vehicle_t class for unmanned vehicles simulations.
 */
#pragma once

#include <cstddef>
#include <memory>
#include <vector>
#include <functional>
#include "process.hpp"

namespace isw::uv {

    class uv_thread_t;
    /** @brief Function type defining a vehicle's control policy, invoked each simulation step. */
    using act = std::function<void(std::shared_ptr< uv_thread_t >)>;
    /** @brief Function type mapping a dimension index to an initial position or velocity value. */
    using fill = std::function<double(size_t)>;

    /**
     * @brief Process representing an unmanned vehicle in the simulation.
     * @details Manages position and velocity vectors across multiple dimensions.
     *   Extends process_t with a factory method for creating fully configured vehicle processes.
     */
    class vehicle_t : public process_t {
        public:
            /**
             * @brief Constructs a vehicle with given dimensionality and initial state functions.
             * @param[in] dimensions Number of spatial dimensions.
             * @param[in] init_pos Function mapping each dimension index to an initial position.
             * @param[in] init_vel Function mapping each dimension index to an initial velocity.
             * @param[in] name Process name, defaults to "default vehicle".
             */
            vehicle_t(size_t dimensions, fill init_pos, fill init_vel, 
                std::string name = "default vehicle");
            
            /**
             * @brief Returns the position along a given dimension.
             * @param[in] idx Dimension index.
             * @return Position value at the given dimension.
             */
            double get_pos(size_t idx);

            /**
             * @brief Returns the velocity along a given dimension.
             * @param[in] idx Dimension index.
             * @return Velocity value at the given dimension.
             */
            double get_vel(size_t idx);

            /**
             * @brief Factory method creating a fully configured vehicle process with its thread.
             * @param[in] dimensions Number of spatial dimensions.
             * @param[in] c_time Thread compute time.
             * @param[in] init_pos Function mapping each dimension index to an initial position.
             * @param[in] init_vel Function mapping each dimension index to an initial velocity.
             * @param[in] policy Control policy invoked each simulation step.
             * @param[in] th_time Thread threshold time, defaults to 0.
             * @param[in] name Process name, defaults to "default vehicle".
             * @return Shared pointer to the created vehicle process.
             */
            static std::shared_ptr<vehicle_t> create_process(size_t dimensions, double c_time, 
                fill init_pos, fill init_vel, act policy, double th_time = 0, 
                std::string name = "default vehicle");

            /**
             * @brief Initializes the vehicle by populating position and velocity vectors.
             * @details Calls process_t::init() then applies init_pos and init_vel to each dimension.
             */
            void init() override;

            /** @brief Position vector, one entry per dimension. */
            std::vector<double> pos;
            /** @brief Velocity vector, one entry per dimension. */
            std::vector<double> vel;

        private:
            /** @brief Position initialization function. */
            fill _init_pos;
            /** @brief Velocity initialization function. */
            fill _init_vel;
    };

    /**
     * @brief Thread implementation for unmanned vehicle control.
     * @details Invokes the configured policy function each simulation step, passing itself as argument.
     */
    class uv_thread_t : public thread_t, public std::enable_shared_from_this<uv_thread_t> {
        public:
            /**
             * @brief Constructs a vehicle thread with a control policy.
             * @param[in] c_time Compute time.
             * @param[in] policy Control policy function.
             * @param[in] th_time Threshold time, defaults to 0.
             */
            uv_thread_t(double c_time, act policy, double th_time = 0);

            /**
             * @brief Executes the vehicle's control policy.
             */
            void fun() override;
        
        private:
            /** @brief Control policy function invoked each step. */
            act _policy;
    };
}