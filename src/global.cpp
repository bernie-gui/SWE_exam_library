/*
 * File: global.cpp
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
 *	This file implements the global_t class methods for managing global state in the simulation system.
 */
#include "global.hpp"
#include <cstddef>

using namespace isw;


global_t::global_t() : _rand( std::make_shared< random_t >() ), _montecarlo_avg(1), _montecarlo_current(1) {}

void global_t::init()
{
    std::fill( _channel_in.begin(), _channel_in.end(), network::channel_t() );
    std::fill( _channel_out.begin(), _channel_out.end(), network::channel_t() );
    std::fill( _montecarlo_current.begin(), _montecarlo_current.end(), 0 );
    // altre cose da inizializzare?
}

std::shared_ptr< random_t > global_t::get_random() { return _rand; }

std::vector< network::channel_t > &global_t::get_channel_in() { return _channel_in; }
std::vector< network::channel_t > &global_t::get_channel_out() { return _channel_out; }

double global_t::get_horizon() const { return _horizon; }
void global_t::set_horizon( double horizon ) { _horizon = horizon; }

size_t global_t::montecarlo_budget() const { return _montecarlo_budget; }
void global_t::set_montecarlo_budget( size_t montecarlo_budget ) { _montecarlo_budget = montecarlo_budget; }

size_t global_t::optimizer_budget() const { return _optimizer_budget; }
void global_t::set_optimizer_budget( size_t optimizer_budget ) { _optimizer_budget = optimizer_budget; }

size_t global_t::network_number() const { return _network_number; }
void global_t::set_network_number( size_t network_number ) { _network_number = network_number; }

double global_t::get_montecarlo_avg( size_t idx ) const { return _montecarlo_avg[idx]; }
void global_t::set_montecarlo_avg( double avg, size_t idx ) { 
    if (idx >= _montecarlo_avg.size()) _montecarlo_avg.resize(idx+1);
    _montecarlo_avg[idx] = avg; 
}

double global_t::montecarlo_current( size_t idx ) const { return _montecarlo_current[idx]; }
void global_t::set_montecarlo_current( double current, size_t idx ) { 
    if (idx >= _montecarlo_current.size()) _montecarlo_current.resize(idx+1);
    _montecarlo_current[idx] = current; 
}

size_t global_t::get_montecarlo_variables() { return _montecarlo_current.size(); }

double global_t::get_optimizer_result() const { return _optimizer_result; }
void global_t::set_optimizer_result( double current ) { _optimizer_result = current; }

std::vector< double > global_t::get_optimizer_optimal_parameters() const { return _optimizer_optimal_parameters; }
void global_t::set_optimizer_optimal_parameters( std::vector< double > current )
{
    _optimizer_optimal_parameters = current;
}

// esempio monitor (dal Tronci)
//  void Global::Monitor1(int item, double amount_available,
//                        double amount_requested, double present_time) {

//   // check for missed sell
//   if (amount_available < amount_requested)
//   // missed sell
//   {

//     _montecarlo_current = _montecarlo_current * ((time_last_missed) / present_time) + (1.0 / present_time);

//     time_last_missed = present_time;

//   }

// } // Monitor1()
