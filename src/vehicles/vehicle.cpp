/*
 * File: vehicle.cpp
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
 *	This file implements the vehicle_t class for unmanned vehicles simulations.
 */
#include "vehicles/vehicle.hpp"
#include <cstddef>
#include <memory>
#include <vector>
#include "process.hpp"

using namespace isw::uv;

vehicle_t::vehicle_t(size_t dimensions, fill init_pos, fill init_vel, std::string name) : 
    process_t(name), pos(dimensions), vel(dimensions), 
    _init_pos(init_pos), _init_vel(init_vel) {}

double vehicle_t::get_pos(size_t idx) {
    return pos[idx];
}

double vehicle_t::get_vel(size_t idx) {
    return vel[idx];
}

void vehicle_t::init() {
    process_t::init();
    for (size_t i = 0; i < pos.size(); i++) {
        pos[i] = _init_pos(i);
        vel[i] = _init_vel(i);
    }
}

std::shared_ptr<vehicle_t> vehicle_t::create_process(size_t dimensions, double c_time, 
    fill init_pos, fill init_vel, act policy, double th_time, std::string name) {
        auto res = std::make_shared<vehicle_t>(dimensions, init_pos, init_vel, name);
        res->add_thread(std::make_shared<uv_thread_t>(c_time, policy, th_time));
        return res;
    }

uv_thread_t::uv_thread_t(double c_time, act policy, double th_time) :
    thread_t(c_time, 0, th_time), _policy(policy) {}

void uv_thread_t::fun() {
    _policy(shared_from_this());
}