/*
 * File: network.cpp
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
 *	This file implements the network_t and scanner_t classes for network operations.
 */
#include "network/network.hpp"
#include <algorithm>
#include "process.hpp"
using namespace isw;

scanner_t::scanner_t( double c_time, double s_time, double th_time ) : thread_t( c_time, s_time, th_time ) {}

void scanner_t::init()
{
    thread_t::init();

    auto system = get_process()->get_system();
    auto &processes = system->get_processes();

    // rebuild scanner
    _scanner.clear();
    for ( size_t i = 0; i < processes.size(); i++ )
        _scanner.push_back( i );
    // Temporary workaround; consider refactoring for production quality
    _current = processes.size();
}

void scanner_t::fun()
{
    auto system = get_process()->get_system();
    auto &processes = system->get_processes();
    if ( processes.size() != _scanner.size() )
    {
        // // rebuild scanner
        // _scanner.clear();
        // for ( size_t i = 0; i < processes.size(); i++ )
        //     _scanner.push_back( i );

        // _current = processes.size();
        init();
    }

    auto global = system->get_global();
    if ( _current >= _scanner.size() )
    {
        auto random = global->get_random();
        std::shuffle( _scanner.begin(), _scanner.end(), random->get_engine() );
        _current = 0;

        on_start_scan();
    }

    auto sched = _scanner[_current];
    _current++;

    auto &current_channel = global->get_channel_out()[sched];
    if ( current_channel.empty() || filter( current_channel ) )
        return;

    auto msg = current_channel.front();
    current_channel.pop();


    assert( msg->sender == sched ); // actually sending to the right one

    global->get_channel_in()[msg->receiver].push( msg );
}

void scanner_t::on_start_scan() {}

bool scanner_t::filter( network::channel_t & /*current_channel*/ ) { return false; }
