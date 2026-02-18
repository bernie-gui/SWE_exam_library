#include <cmath>
#include <iostream>
#include <memory>
#include <vector>
#include "global.hpp"
#include "io/lambda_parser.hpp"
#include "io/output_writer.hpp"
#include "montecarlo.hpp"
#include "process.hpp"
#include "simulator.hpp"
#include "system.hpp"
#include "utils/rate.hpp"
#include "utils/vehicles/functions.hpp"
#include "utils/vehicles/vehicle.hpp"

using namespace isw;

// Global state to hold simulation parameters
class exam_global : public global_t
{
public:
    double T;
    double H;
    double M;
    size_t N;
    double L;
    double V;
    double A;
    double D;

    utils::rate_meas_t measure;

    void init() override
    {
        global_t::init();
        measure.init();
    }
};

// Custom system to handle collision detection at end of each step
class exam_system : public system_t
{
public:
    exam_system( std::shared_ptr< global_t > global ) : system_t( global ) {}

    void on_end_step() override
    {
        auto gl = get_global< exam_global >();

        // get all vehicle processes
        auto &vehicles = get_processes< uv::vehicle_t >( "UAVs" );

        // count collisions
        size_t collisions = uv::count_collisions( vehicles, gl->D );

        // update measurement
        gl->measure.update( collisions, get_current_time() );
    }
};

// Simulator wrapper to handle Montecarlo data collection
class exam_simulator : public simulator_t
{
public:
    exam_simulator( std::shared_ptr< system_t > sys ) : simulator_t( sys ) {}

    void on_terminate() override
    {
        auto gl = get_global< exam_global >();
        gl->set_montecarlo_current( gl->measure.get_rate() );
    }
};

int main()
{
    auto gl = std::make_shared< exam_global >();

    // Parse parameters
    auto reader = lambda_parser_t( "examples/example_09_25_1.txt",
                                   { { "T", [gl]( auto &iss ) { iss >> gl->T; } },
                                     { "H",
                                       [gl]( auto &iss )
                                       {
                                           iss >> gl->H;
                                           gl->set_horizon( gl->H );
                                       } },
                                     { "M",
                                       [gl]( auto &iss )
                                       {
                                           iss >> gl->M;
                                           gl->set_montecarlo_budget( gl->M );
                                       } },
                                     { "N", [gl]( auto &iss ) { iss >> gl->N; } },
                                     { "L", [gl]( auto &iss ) { iss >> gl->L; } },
                                     { "V", [gl]( auto &iss ) { iss >> gl->V; } },
                                     { "A", [gl]( auto &iss ) { iss >> gl->A; } },
                                     { "D", [gl]( auto &iss ) { iss >> gl->D; } } } );
    reader.parse();

    // Use custom system
    auto sys = std::make_shared< exam_system >( gl );

    // Initial position uniform in [-L, L]
    auto init_pos = [gl]( size_t ) { return gl->get_random()->uniform_range( -gl->L, gl->L ); };

    // Initial velocity 0
    auto init_vel = []( size_t ) { return 0.0; };

    // Vehicle behavior policy
    auto policy = [gl]( std::shared_ptr< uv::uv_thread_t > th )
    {
        auto veh = th->get_process< uv::vehicle_t >();

        for ( size_t k = 0; k < 3; ++k )
        {
            // Calculate probability p based on current position
            double p = std::exp( -gl->A * ( veh->pos[k] + gl->L ) / ( 2.0 * gl->L ) );

            // Limit probability
            if ( p > 1.0 )
                p = 1.0;
            if ( p < 0.0 )
                p = 0.0;

            double rnd = gl->get_random()->uniform_range( 0.0, 1.0 );
            if ( rnd < p )
            {
                veh->vel[k] = gl->V;
            }
            else
            {
                veh->vel[k] = -gl->V;
            }

            // Update position for next step (Eq 1)
            veh->pos[k] += veh->vel[k] * gl->T;
        }
    };

    // Create N vehicles
    for ( size_t i = 0; i < gl->N; ++i )
    {
        auto veh_proc = uv::vehicle_t::create_process( 3,     // dimensions
                                                       gl->T, // compute time / period
                                                       init_pos, init_vel, policy,
                                                       gl->T, // thread time / period
                                                       "uav_" + std::to_string( i ) );
        sys->add_process( veh_proc, "UAVs" );
    }

    // No need for collision detector process anymore

    auto sim = std::make_shared< exam_simulator >( sys );
    auto monty = montecarlo_t::create( sim );

    std::cout << "Starting simulation..." << std::endl;
    monty->run();
    std::cout << "Simulation ended." << std::endl;

    // Write results
    // output_writer_t twriter( "results_09_25_1.txt" );
    // twriter.write_line( "C " + std::to_string( gl->get_montecarlo_avg() ) );
    std::cout << "C " << gl->get_montecarlo_avg() << std::endl;

    return 0;
}
