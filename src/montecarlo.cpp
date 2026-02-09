/*
 * File: montecarlo.cpp
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
 *	This file implements the montecarlo_t class methods for running Monte Carlo simulations.
 */
#include "montecarlo.hpp"
#include "simulator.hpp"
using namespace isw;

montecarlo_t::montecarlo_t( std::shared_ptr< simulator_t > sim ) : _sim( sim ) {}

std::shared_ptr< simulator_t > montecarlo_t::get_simulator() const { return _sim; }

void montecarlo_t::run()
{
    auto global = _sim->get_system()->get_global();
    global->set_montecarlo_avg( 0.0 );
    for ( size_t i = 0; i < global->montecarlo_budget(); i++ )
    {
        _sim->run();

        double local_value = global->get_montecarlo_avg() * ( i / static_cast< double >( i + 1 ) ) +
            global->montecarlo_current() / static_cast< double >( i + 1 );
        global->set_montecarlo_avg( local_value );
    }
}
std::shared_ptr< montecarlo_t > montecarlo_t::create( const std::shared_ptr< simulator_t > sim )
{
    return std::shared_ptr< montecarlo_t >( new montecarlo_t( sim ) );
}
