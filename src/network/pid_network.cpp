/*
 * File: pid_network.cpp
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
 *	This file implements the pid_scanner_t thread for network operations.
 */
#include "network/pid_network.hpp"
#include "network/network.hpp"
using namespace isw::network;

pid_scanner_t::pid_scanner_t(double obj_occupancy, double th_time) : scanner_t(0, S_TIME_MIN, th_time), 
    _obj_occupancy(obj_occupancy), _integral(0), _prev_meas(0), _prev_dv(0) {}

void pid_scanner_t::on_start_scan() {

}

void pid_scanner_t::init() {

}