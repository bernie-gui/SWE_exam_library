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

//TODO: add documentation
namespace isw::uv {

    class vehicle_t;
    using act = std::function<void(std::shared_ptr< vehicle_t >)>;
    using fill = std::function<double(size_t)>;

    class vehicle_t : public process_t {
        public:
            vehicle_t(size_t dimensions, fill init_pos, fill init_vel, 
                std::string name = "default vehicle"); //more than one environment???
            
            double get_pos(size_t idx);

            double get_vel(size_t idx);

            static std::shared_ptr<vehicle_t> create_process(size_t dimensions, double c_time, 
                fill init_pos, fill init_vel, act policy, double th_time = 0, 
                std::string name = "default vehicle");

            void init() override;

            std::vector<double> pos, vel;

        private:
            fill _init_pos, _init_vel;
    };

    class uv_thread_t : public thread_t {
        public:
            uv_thread_t(double c_time, act policy, double th_time = 0);

            void fun() override;
        
        private:
            act _policy;
    };
}