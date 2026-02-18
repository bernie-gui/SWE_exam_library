#include <cmath>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <vector>
#include "global.hpp"
#include "io/lambda_parser.hpp"
#include "io/output_writer.hpp"
#include "montecarlo.hpp"
#include "simulator.hpp"
#include "system.hpp"
#include "utils/backtracking.hpp"
#include "utils/rate.hpp"
#include "utils/vehicles/functions.hpp"
#include "utils/vehicles/vehicle.hpp"

using namespace isw;

// Global state to hold simulation parameters and cache
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

    // Cached positions for the current step
    // Key: vehicle relative ID (or absolute ID), Value: Position vector
    std::map< size_t, std::vector< double > > cached_positions;

    void init() override
    {
        global_t::init();
        measure.init();
        cached_positions.clear();
    }
};

// Custom system to handle caching and collision detection
class exam_system : public system_t
{
public:
    exam_system( std::shared_ptr< global_t > global ) : system_t( global ) {}

    // Override step to update cache at the beginning of the step
    void step() override
    {
        auto gl = get_global< exam_global >();
        gl->cached_positions.clear();

        // Cache positions of all UAVs BEFORE they move
        try
        {
            auto &vehicles = get_processes< uv::vehicle_t >( "UAVs" );
            for ( auto &veh : vehicles )
            {
                // We use relative ID for simplicity as it's stable 0..N-1
                if ( auto id = veh->get_relative_id() )
                {
                    gl->cached_positions[*id] = veh->pos;
                }
            }
        }
        catch ( ... )
        {
            // Handle case where UAVs might not be registered yet or empty
        }

        // Run standard step (processes will use cached_positions)
        system_t::step();
    }

    void on_end_step() override
    {
        auto gl = get_global< exam_global >();
        auto &vehicles = get_processes< uv::vehicle_t >( "UAVs" );
        size_t collisions = uv::count_collisions( vehicles, gl->D );
        gl->measure.update( collisions, get_current_time() );
    }
};

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
    auto reader = lambda_parser_t( "examples/example_09_25_2.txt",
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

    auto sys = std::make_shared< exam_system >( gl );

    auto init_pos = [gl]( size_t ) { return gl->get_random()->uniform_range( -gl->L, gl->L ); };

    auto init_vel = []( size_t ) { return 0.0; };

    // Exam 2 Policy (USING CACHE)
    auto policy = [gl]( std::shared_ptr< uv::uv_thread_t > th )
    {
        auto veh = th->get_process< uv::vehicle_t >();

        // Define the parameter space: 3 dimensions, options {V, -V}
        std::unordered_set< double > options = { gl->V, -gl->V };
        std::vector< std::unordered_set< double > > ranges( 3, options );

        // Define objective function to MINIMIZE (using CACHE)
        auto objective_func = [gl, veh]( std::shared_ptr< std::vector< double > > v ) -> double
        {
            double d_val = 0;
            // Iterate over CACHED positions
            for ( auto const &[id, pos] : gl->cached_positions )
            {
                double dist_sq = 0;
                for ( size_t k = 0; k < 3; ++k )
                {
                    // v is the candidate velocity vector
                    double next_pos_k = veh->pos[k] + ( *v )[k] * gl->T;
                    double diff = next_pos_k - pos[k]; // Use cached pos
                    dist_sq += diff * diff;
                }
                d_val += std::exp( -dist_sq / ( 2.0 * gl->L ) );
            }
            return d_val;
        };

        // Use backtracking utility to find optimal velocities
        auto best_bucket = utils::arg_min_max< double, double >( ranges, objective_func, utils::arg_strat::MIN );

        // Select one uniformly at random
        auto chosen_vel = utils::get_unif_random( best_bucket, gl->get_random()->get_engine() );

        // Apply velocity and update position
        veh->vel = *chosen_vel;
        for ( size_t k = 0; k < 3; ++k )
        {
            veh->pos[k] += veh->vel[k] * gl->T;
        }
    };

    for ( size_t i = 0; i < gl->N; ++i )
    {
        auto veh_proc =
            uv::vehicle_t::create_process( 3, gl->T, init_pos, init_vel, policy, gl->T, "uav_" + std::to_string( i ) );
        sys->add_process( veh_proc, "UAVs" );
    }

    auto sim = std::make_shared< exam_simulator >( sys );
    auto monty = montecarlo_t::create( sim );

    std::cout << "Starting simulation (With Cache)..." << std::endl;
    monty->run();
    std::cout << "Simulation ended." << std::endl;

    // output_writer_t twriter( "results_09_25_2.txt" );
    std::cout << "O " << gl->get_optimizer_result() << std::endl;
    // twriter.write_line( "O " + std::to_string( gl->get_optimizer_result() ) );

    // Print optimal param
    auto opt_params = gl->get_optimizer_optimal_parameters();
    if ( !opt_params.empty() )
    {
        // V_x
        // twriter.write_line( "V " + std::to_string( opt_params[0] ) );
        std::cout << "V " << opt_params[0] << std::endl;
    }

    return 0;
}
