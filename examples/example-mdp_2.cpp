/*
 * File: example-09-01-2025_2.cpp
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
 *	This example demonstrates an alternative usage of the simulation library for the scenario on January 9, 2025.
 */
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <tuple>
#include <vector>
#include "global.hpp"
#include "io/input_parser.hpp"
#include "montecarlo.hpp"
#include "process.hpp"
#include "simulator.hpp"
#include "system.hpp"

using namespace isw;

class mdp_global : public global_t
{
public:
    size_t current_state; // i
    double total_cost, cost_limit;
    std::vector< std::vector< std::tuple< double, double > > > matrix;

    std::map< double, size_t > cost_freq;

    void init() override
    {
        global_t::init();
        current_state = 0;
        total_cost = 0;
    }

    void endMonitor()
    {
        cost_freq[total_cost]++;

        std::cout << ( total_cost <= cost_limit ) << ", " << total_cost << "-" << cost_limit << std::endl;
        set_montecarlo_current( total_cost <= cost_limit ? 1 : 0 );
    };
};

class my_input_parser_t : public input_parser_t
{
    // M[i][j] = <prob i -> j; cost>
public:
    my_input_parser_t( std::shared_ptr< mdp_global > global ) :
        input_parser_t( global, "examples/example-mdp_2.txt" )
    {
    }

    size_t n_states;
    void parse() override
    {
        size_t i, j;
        double prob, cost;
        std::string line;
        auto &stream = get_stream();

        //                       ⚡⚡⚡⚡⚡⚡⚡⚡⚡⚡
        auto casted = get_global< mdp_global >();
        casted->set_montecarlo_budget( 1000 );

        while ( std::getline( stream, line ) )
        {
            std::istringstream iss( line );
            char tag;
            iss >> tag;

            switch ( tag )
            {
                case 'C':
                {
                    iss >> casted->cost_limit;
                }
                break;

                case 'N':
                {
                    iss >> n_states;
                    casted->matrix.resize( n_states );
                    for ( size_t i = 0; i < n_states; ++i )
                        casted->matrix[i].resize( n_states );

                    casted->matrix[n_states - 1][n_states - 1] = std::tuple< double, double >( 1.0, 0.0 );
                }
                break;
                case 'A':
                {
                    iss >> i >> j >> prob >> cost;
                    casted->matrix[i][j] = std::tuple< double, double >( prob, cost );
                }
                break;
                default:
                    break;
            }
        }
    }
};

class markov_thread : public thread_t
{
public:
    void fun() override
    {

        auto global = get_global< mdp_global >();
        auto random = global->get_random();


        double random_value = random->uniform_range( 0.0, 1.0 );

        // [1,2,3]
        // [0.2 0.1 0.7]
        // [0 ; 0.2 ] 1
        // [0.2 ; 0.3] 2
        // [0.3 : 1] 3
        double sum = 0.0;
        size_t i = global->current_state;

        for ( size_t j = 0; j < global->matrix.size(); ++j )
        {
            auto prob = std::get< 0 >( global->matrix[i][j] );
            auto cost = std::get< 1 >( global->matrix[i][j] );

            if ( prob > 0.0 && random_value >= sum && random_value < sum + prob )
            {
                global->current_state = j;
                // adding cost to the simulation
                global->total_cost += cost;
                return;
            }

            sum += prob;
        }
    }

    markov_thread() : thread_t( 1.0, 0.0 ) {}
};

class markov_simulator : public simulator_t
{
public:
    using simulator_t::simulator_t;

    void on_terminate() override
    {
        auto global = get_global< mdp_global >();
        std::dynamic_pointer_cast< mdp_global >( global )->endMonitor();
    }

    virtual bool should_terminate() override
    {
        auto global = get_global< mdp_global >();
        bool check = global->current_state == global->matrix.size() - 1;
        return check;
    };
};

int main()
{
    auto global = std::make_shared< mdp_global >();
    my_input_parser_t parser( global );
    parser.parse();

    auto system = system_t::create( global, "mdp_system" );
    auto process = process_t::create();
    process->add_thread( std::make_shared< markov_thread >() );
    system->add_process( process );
    process->set_system( system );

    auto simulator = std::make_shared< markov_simulator >( system );
    auto montecarlino = montecarlo_t::create( simulator );

    montecarlino->run();
    std::cout << "p: " << global->get_montecarlo_avg() << std::endl;

    for ( const auto &[cost, count] : global->cost_freq )
        std::cout << cost << " : " << count << " times\n";
}
