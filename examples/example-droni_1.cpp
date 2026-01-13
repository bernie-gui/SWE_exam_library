/*
 * File: main.cpp
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
 */
#include <cassert>
#include <cmath>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include "io/input_parser.hpp"
#include "io/output_writer.hpp"
#include "process.hpp"
#include "simulator.hpp"
#include "system.hpp"

using namespace isw;

class drone_global_t : public global_t
{
public:
    drone_global_t() : global_t() {}
    double X1, X2, Y1, Y2, Z1, Z2;
    double alpha;
    double N;
    double R;

    double P;
    double Q;
};

class droni_input_parser_t : public input_parser_t
{
public:
    droni_input_parser_t( std::shared_ptr< global_t > global ) : input_parser_t( global, "examples/example-droni_1.txt" )
    {
    }

    void parse()
    {
        auto global = get_global< drone_global_t >();
        auto &stream = get_stream();
        std::string line;
        char tag;

        std::getline( stream, line );
        std::istringstream iss( line );
        double horizon;
        iss >> tag >> horizon;
        global->set_horizon( horizon );

        std::getline( stream, line );
        iss = std::istringstream( line );
        iss >> tag >> global->N;

        std::getline( stream, line );
        iss = std::istringstream( line );
        iss >> tag >> global->alpha;

        std::getline( stream, line );
        iss = std::istringstream( line );
        iss >> tag >> global->R;


        std::getline( stream, line );
        iss = std::istringstream( line );
        iss >> global->X1 >> global->X2 >> global->Y1 >> global->Y2 >> global->Z1 >> global->Z2;
    }
};


double distance( double x1, double x2, double y1, double y2, double z1, double z2 )
{
    return std::sqrt( std::pow( x1 - x2, 2 ) + std::pow( y1 - y2, 2 ) + std::pow( z1 - z2, 2 ) );
}


class drone_process_t : public process_t
{
public:
    double x, y, z;
    drone_process_t() : process_t( "my_epic_drone" ), x( 0 ), y( 0 ), z( 0 ) {}

    void init() override
    {
        auto global = get_global< drone_global_t >();
        auto random = global->get_random();
        x = random->uniform_range( global->X1, global->X2 );
        y = random->uniform_range( global->Y1, global->Y2 );
        z = random->uniform_range( global->Z1, global->Z2 );
    }
};

class drone_thread_t : public thread_t
{
public:
    drone_thread_t() : thread_t( 1, 0, 0 ) {}

    void fun() override
    {
        auto global = get_global< drone_global_t >();
        auto process = get_process< drone_process_t >();
        auto random = global->get_random();

        double v_x, v_y, v_z;
        double alpha = global->alpha;

        v_x = random->uniform_range( -alpha, alpha );
        v_y = random->uniform_range( -alpha, alpha );
        v_z = random->uniform_range( -alpha, alpha );

        process->x =
            std::min< double >( global->X2, std::max< double >( global->X1, process->x + v_x * get_compute_time() ) );
        process->y =
            std::min< double >( global->Y2, std::max< double >( global->Y1, process->y + v_y * get_compute_time() ) );
        process->z =
            std::min< double >( global->Z2, std::max< double >( global->Z1, process->z + v_z * get_compute_time() ) );
    }
};

class drone_system_t : public system_t
{
public:
    using system_t::system_t;

    void init() override
    {
        system_t::init();
        auto global = get_global< drone_global_t >();
        global->Q = global->N;
    }

    void on_end_step() override
    {
        std::vector<size_t> to_remove_ids;
        auto global = get_global< drone_global_t >();
        for ( auto &drone1 : get_processes< drone_process_t >( "drone" ) )
        {
            for ( auto &drone2 : get_processes< drone_process_t >( "drone" ) )
            {
                if ( drone1 == drone2 || not drone1->is_active() || not drone2->is_active() )
                {
                    continue;
                }

                // std::cout << "drone" << drone1->get_id().value() << "(x,y,z)=(" << drone1->x << ", " << drone1->y
                //           << ", " << drone1->z << ")\n"
                //           << "drone" << drone2->get_id().value() << "(x,y,z)=(" << drone2->x << ", " << drone2->y
                //           << ", " << drone2->z << ")\n";

                double R = global->R;
                if ( distance( drone1->x, drone2->x, drone1->y, drone2->y, drone1->z, drone2->z ) <= R )
                {
                    to_remove_ids.push_back(drone1->get_id().value());
                }
            }
        }
        for ( auto id : to_remove_ids )
        {
            auto drone = get_processes< drone_process_t >( "drone" ).at( id );
            if ( drone->is_active() )
            {
                drone->set_active( false );
                global->Q -= 1;
            }
        }
    }
};

class my_simulator_t : public simulator_t
{
public:
    my_simulator_t( const std::shared_ptr< system_t > sys ) : simulator_t( sys ) {}


    bool should_terminate() override
    {
        auto system = get_system();
        auto global = get_global();

        return system->get_current_time() >= global->get_horizon();
    }

    void on_terminate() override
    {
        auto global = get_global< drone_global_t >();
        global->P = (double) global->Q / global->N;
    }
};

int main()
{
    auto global = std::make_shared< drone_global_t >();
    auto writer = output_writer_t( "examples/example-droni_1.txt" );
    writer.write_line( "2025-01-09-AntonioMario-RossiPatrizio-1234567" );

    droni_input_parser_t parser( global );
    parser.parse();

    auto system = std::make_shared< drone_system_t >( global );
    for ( size_t i = 0; i < global->N; ++i )
    {
        auto process = std::make_shared< drone_process_t >();
        process->add_thread( std::make_shared< drone_thread_t >() );
        system->add_process( process, "drone" );
    }

    auto simulator = std::make_shared< my_simulator_t >( system );
    simulator->run();

    writer << "Q " << global->Q << std::endl << "N " << global->N << std::endl << "P " << global->P << std::endl;
}
