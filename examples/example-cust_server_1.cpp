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
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <tuple>
#include <vector>
#include "global.hpp"
#include "io/input_parser.hpp"
#include "montecarlo.hpp"
#include "network/message.hpp"
#include "process.hpp"
#include "simulator.hpp"
#include "system.hpp"

using namespace isw;

class my_global_t : public global_t
{
public:
    double mean = 3600, stddev = 500;
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

class monitor_thread_t : public thread_t
{
public:
    monitor_thread_t() : thread_t( 1, 1 ) {};

    double last_time_stamp = 0.0;
    size_t i = 1;

    void fun() override
    {
        // std::cout << "I RANNNNNNNNNNN\n";
        auto global = get_global< my_global_t >();
        auto front_msg = receive_message();
        if ( !front_msg )
            return;
        double current_timestamp = front_msg->timestamp;
        double difference = current_timestamp - last_time_stamp;
        // std::cout << difference << std::endl;
        // std::cout << get_process()->get_system()->current_time() << std::endl;

        if ( current_timestamp == 0 )
        {
            last_time_stamp = current_timestamp;
            return;
        }
        else
        {
            if ( i % 1000 == 0 )
            {
                std::cout << global->get_montecarlo_avg() << " " << last_time_stamp / current_timestamp << " "
                          << difference / current_timestamp << " " << difference << std::endl;
            }
            global->set_montecarlo_avg( global->get_montecarlo_avg() * ( static_cast< double >( i ) / ( i + 1 ) ) +
                                        difference / i );
            i++;

            last_time_stamp = current_timestamp;
        }
    }
};

class my_simulator_t : public simulator_t
{
public:
    using simulator_t::simulator_t;

    bool should_terminate()
    {
        auto system = get_system();
        return system->get_current_time() >= 10000000;
    }
};

int main( void )
{
    auto my_global = std::make_shared< my_global_t >();
    auto system = system_t::create( my_global, "customer_monitor_system" );
    // init
    auto customer_process = process_t::create( "customer" );
    customer_process->add_thread( std::make_shared< cust_thread_t >() );
    auto monitor_process = process_t::create( "monitor" );
    monitor_process->add_thread( std::make_shared< monitor_thread_t >() );
    // init standard network
    system = system->add_network( 1000, 1 )->add_process( monitor_process, "monitor" )->add_process( customer_process );

    // auto montecarlito = montecarlo_t::create< my_simulator_t >( system );
    auto simulator = std::make_shared< my_simulator_t >( system );
    simulator->run();

    std::cout << "Average time difference: " << my_global->get_montecarlo_avg() << std::endl;
    return 0;
}
