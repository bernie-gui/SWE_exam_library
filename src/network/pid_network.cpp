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
#include <algorithm>
using namespace isw;

pid_scanner_t::pid_scanner_t(double obj_occupancy, double th_time, double error_threshold) : 
    scanner_t(0.2, S_TIME_MIN, th_time), _obj_occupancy(obj_occupancy), _integral(0), 
    _prev_error(0), _prev_dv(0), _last_time(0), _error_threshold(error_threshold) {}

void pid_scanner_t::on_start_scan() {
    if (get_thread_time() == 0) return;
    auto gl = get_global();
    auto queues = gl->get_channel_out();
    double measurement = 0;
    for (auto &c : queues) 
        measurement += static_cast<double>(c.size()) / queues.size();
    double error = measurement - _obj_occupancy;
    double dt = get_thread_time() - _last_time;
    double dv = (error - _prev_error) / dt; //update last time
    double smooth_dv = (1 - DV_ALPHA) * _prev_dv + DV_ALPHA * dv;
    double control_1 = KP * error + KD * smooth_dv;
    if (std::abs(error) < _error_threshold) _integral = 0.0;
    else {
        double try_integral = _integral + error * dt;
        double try_control = control_1 + try_integral * KI;
        double try_sleep = get_sleep_time() - try_control;
        if (try_sleep > S_TIME_MIN && try_sleep < S_TIME_MAX)
            _integral = try_integral;
    }
    double control = control_1 + _integral * KI;
    set_sleep_time(std::clamp( get_sleep_time() - control, 
        S_TIME_MIN, S_TIME_MAX));
    _prev_error = error;
    _prev_dv = smooth_dv;
    _last_time = get_thread_time();
}

void pid_scanner_t::init() {
    scanner_t::init();
    _integral = 0;
    _prev_error = 0;
    _prev_dv = 0;
    _last_time = 0;
    set_sleep_time(S_TIME_MIN);
}