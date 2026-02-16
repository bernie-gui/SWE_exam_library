/*
 * File: rate.cpp
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
 *	This file implements the rate_meas_t class methods for rate measurements in the simulation system.
 */
#include <stdexcept>
#include "utils/rate.hpp"
using namespace isw::utils;

rate_meas_t::rate_meas_t(): _rate(0), _last_denom(0), _updated(false) {}

void rate_meas_t::update(double amount, double denom) {
    if (!denom) 
        throw std::runtime_error("Math error in rate measurement: update method called at time zero");
    _rate = _rate * ( _last_denom / denom ) + amount / denom;
    _last_denom = denom;
    if (!_updated) _updated = true;
}

void rate_meas_t::update(double denom) {
    update(0, denom);
}

void rate_meas_t::increase_amount(double amount) {
    update(amount, _last_denom);
}

void rate_meas_t::increase_denom(double increase) {
    update(0, _last_denom + increase);
}

bool rate_meas_t::was_updated() { return  _updated;}

void rate_meas_t::init() {
    _rate = _last_denom = 0;
    _updated = false;
}

double rate_meas_t::get_rate() {return _rate;}