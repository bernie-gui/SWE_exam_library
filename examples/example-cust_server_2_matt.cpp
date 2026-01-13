/*
 * File: example-09-01-2025_3.cpp
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
 *	This example demonstrates another variation of the simulation library usage for the January 9, 2025 scenario.
 */
#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <tuple>
#include <vector>
#include "global.hpp"
#include "io/input_parser.hpp"
#include "montecarlo.hpp"
#include "network/message.hpp"
#include "network/network.hpp"
#include "process.hpp"
#include "simulator.hpp"
#include "system.hpp"

using namespace isw;

class my_global_t : public global_t
{
public:
    double mean = 3600, stddev = 500;
    size_t N = 100;
    // si :)
};

class cust_thread_t : public thread_t
{
public:
    void fun() override
    {

        /// la mia roba
        auto global = get_global< my_global_t >();
        auto random = global->get_random();

        network::message_t msg;
        send_message( "monitor", 0, msg );
        set_sleep_time( random->gaussian_sample( global->mean, global->stddev ) );
        // std::cout << get_sleep_time() << "  " << get_thread_time() << std::endl;
    }


    cust_thread_t() : thread_t( 0, 0 ) {}
};

bool global_flag = true;

class monitor_t : public thread_t
{
public:
    std::vector< size_t > message_counter;
    size_t total_sum = 0;
    std::vector< network::message_t > vect;

    void init() override
    {
        thread_t::init();
        auto global = get_global< my_global_t >();
        message_counter = std::vector< size_t >( global->N, 0 );

        vect = std::vector< network::message_t >( 0 );
    };


    void fun() override
    {
        if ( global_flag )
        {
            global_flag = false;
            std::sort( vect.begin(), vect.end(), []( auto &a, auto &b ) { return a.timestamp < b.timestamp; } );

            for ( auto &p : vect )
            {
                std::cout << p.timestamp << std::endl;
            }
            std::cout << "end of batch" << std::endl;
        }
        vect.clear();


        auto message = receive_message();
        if ( !message )
        {
            return;
        }
        vect.push_back( *message );
        message_counter[message->sender_rel] += 1;
        total_sum += 1;
    }
};

class my_simulator_t : public simulator_t
{
public:
    using simulator_t::simulator_t;

    bool should_terminate()
    {
        auto system = get_system();
        return system->get_current_time() >= 1000;
    }
};

class ordered_scanner_t : public scanner_t
{
public:
    ordered_scanner_t( double c_time, double s_time, double th_time ) : scanner_t( c_time, s_time, th_time ) {};
    void on_start_scan()
    {
        auto system = get_process()->get_system();
        auto global = system->get_global();

        /* Getting the First Time Stamp Available */
        size_t current_id = 0;
        global_flag = true;
        while ( current_id < _scanner.size() )
        {
            auto &current_channel = global->get_channel_out()[current_id];
            if ( current_channel.empty() )
            {
                continue;
            }
            auto &msg = current_channel.front();
            first_timestamp_seen = msg->timestamp;
        }

        // if we get here everything was empty; we default to infity so we skip everything

        first_timestamp_seen = std::numeric_limits< double >::infinity();
    };
    bool filter( network::channel_t &current_channel )
    {
        auto &msg = current_channel.front();
        if ( msg->timestamp > first_timestamp_seen )
        {
            return false;
        }
        return true;
    };

    double first_timestamp_seen;
};

int main( void )
{
    auto my_global = std::make_shared< my_global_t >();
    auto system = system_t::create( my_global, "customer_monitor_system" );
    // init
    auto customer_process = process_t::create( "customer" );
    customer_process->add_thread( std::make_shared< cust_thread_t >() );
    auto monitor_process = process_t::create( "monitor" );
    auto thread = std::make_shared< monitor_t >();
    monitor_process->add_thread( thread );


    // network_t ordered_network = network_t::create();
    auto ordered_network = std::make_shared< network_t >();
    ordered_network->add_thread( std::make_shared< ordered_scanner_t >( 0.1, 1, 0 ) );
    system->add_network( ordered_network );

    system = system->add_process( monitor_process, "monitor" )->add_process( customer_process );

    // auto montecarlito = montecarlo_t::create< my_simulator_t >( system );
    auto simulator = std::make_shared< my_simulator_t >( system );
    simulator->run();

    for ( size_t i = 0; i < thread->message_counter.size(); i++ )
    {
        std::cout << i << " " << thread->message_counter[i];
    }
    std ::cout << "M1" << thread->total_sum << std::endl;
    return 0;
}
