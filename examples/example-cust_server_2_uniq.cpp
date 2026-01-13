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
    size_t N = 100;
    // si :)
};

class cust_thread_t : public thread_t
{
public:
    void fun() override
    {
        // std::cout << "i was called :) \n";

        /// la mia roba
        auto global = get_global< my_global_t >();
        auto random = global->get_random();

        network::message_t msg;
        send_message( "dispatcher", 0, msg );
        set_sleep_time( random->gaussian_sample( global->mean, global->stddev ) );
        // std::cout << get_sleep_time() << "  " << get_thread_time() << std::endl;
    }


    cust_thread_t() : thread_t( 0, 0 ) {}
};

class dispatcher_thread_t : public thread_t
{
public:
    struct CompareMessages
    {
        bool operator()( const std::shared_ptr< network::message_t > &a,
                         const std::shared_ptr< network::message_t > &b ) const
        {
            return a->timestamp < b->timestamp;
        }
    };

    std::vector< size_t > msg_counter;
    std::multiset< std::shared_ptr< network::message_t >, CompareMessages > msg_buffer;

    dispatcher_thread_t() : thread_t( 1, 0 ) {}

    void init() override
    {
        thread_t::init();
        auto global = get_global< my_global_t >();
        msg_counter = std::vector< size_t >( global->N, 0 );
    }

    void fun() override
    {
        // get message
        auto message = receive_message();
        if ( !message )
        {
            return;
        }
        msg_counter[message->sender_rel] += 1;
        msg_buffer.insert( message );
    };
};

class my_simulator_t : public simulator_t
{
public:
    using simulator_t::simulator_t;

    bool should_terminate()
    {
        auto system = get_system();
        // return system->current_time() >= 1000000;
        return system->get_current_time() >= 10000;
    }
};

int main( void )
{
    auto my_global = std::make_shared< my_global_t >();
    auto system = system_t::create( my_global, "customer_monitor_system" );
    // init
    auto disp_process = process_t::create( "dispatcher" );
    auto disp_thread = std::make_shared< dispatcher_thread_t >();
    disp_process->add_thread( disp_thread );
    // init standard network
system = system->add_network( .1, 0 )->add_process( disp_process, "dispatcher" );
    for ( size_t i = 0; i < my_global->N; i++ )
    {
        auto cust_process = process_t::create( "customer_" + std::to_string( i ) );
        auto cust_thread = std::make_shared< cust_thread_t >();
        cust_process->add_thread( cust_thread );
        system = system->add_process( cust_process );
    }

    // auto montecarlito = montecarlo_t::create< my_simulator_t >( system );
    auto simulator = std::make_shared< my_simulator_t >( system );
    simulator->run();

    // simulate results.txt
    int counter = 0;
    for ( size_t i = 1; i <= my_global->N; i++ )
    {
        std::cout << i << " " << disp_thread->msg_counter[i - 1] << std::endl;
        counter += disp_thread->msg_counter[i - 1];
    }
    std::cout << "M1 " << counter << std::endl;
    for (auto& message : disp_thread->msg_buffer){
        std::cout << "T[" << message->timestamp << "s]: " << message->sender << " -> " << message -> receiver << " relative:" << message->sender_rel << std::endl;
    }
    return 0;
}
