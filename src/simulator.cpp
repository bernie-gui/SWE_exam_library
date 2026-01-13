/*
 * File: simulator.cpp
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
 *	This file implements the simulator_t class methods for running simulations.
 */
#include "simulator.hpp"

using namespace isw;

// copy constructor goes here :)

simulator_t::simulator_t( const std::shared_ptr< system_t > sys ) : _system( sys ) {}

void simulator_t::run()
{
    _system->init();
    while ( !should_terminate() )
    {
        _system->step();
    }
    on_terminate();
}

bool simulator_t::should_terminate() {
    return _system->get_current_time() >= _system->get_global()->get_horizon();
}

void simulator_t::on_terminate() {}

std::shared_ptr< system_t > simulator_t::get_system() { return _system; }
