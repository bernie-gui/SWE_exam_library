#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "common.hpp"
#include "global.hpp"
#include "process.hpp"
#include "random.hpp"
#include "simulator.hpp"
#include "system.hpp"

#include "montecarlo.hpp"
#include "optimizer.hpp"

#include "io/input_parser.hpp"
#include "io/logger.hpp"
#include "io/output_writer.hpp"

#include "network/message.hpp"
#include "network/network.hpp"

// Per compatibilità cross-platform
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace isw;

class my_global_t : public global_t
{
public:
    my_global_t() : global_t() {}

    double horizon = 0.0;
    size_t n_vehicles = 0;
    double velocity = 0.0;
    double time_step = 0.0;
    double radius = 0.0;

    // Double buffering posizioni
    std::vector< std::pair< double, double > > pos_current;
    std::vector< std::pair< double, double > > pos_next;

    // Stato collisioni
    std::vector< bool > collision_state;

    // Accumulatore collisioni run corrente
    size_t current_run_collisions = 0;

    void init() override
    {
        global_t::init();
        current_run_collisions = 0;

        if ( n_vehicles > 0 )
        {
            pos_current.resize( n_vehicles );
            pos_next.resize( n_vehicles );
            collision_state.assign( n_vehicles, false );
        }
    }
};

class my_input_parser_t : public input_parser_t
{
public:
    my_input_parser_t( std::shared_ptr< global_t > global ) : input_parser_t( global, "parameters.txt" ) {}

    void parse() override
    {
        auto global = get_global< my_global_t >();
        std::ifstream &in = get_stream();
        std::string key;

        while ( in >> key )
        {
            if ( key == "H" )
                in >> global->horizon;
            else if ( key == "N" )
                in >> global->n_vehicles;
            else if ( key == "V" )
                in >> global->velocity;
            else if ( key == "T" )
                in >> global->time_step;
            else if ( key == "R" )
                in >> global->radius;
        }
    }
};

class my_thread_t : public thread_t
{
public:
    my_thread_t() : thread_t( 0.0, 0.0, 0.0 ), x( 0.0 ), y( 0.0 ) {}

    double x;
    double y;

    void init() override
    {
        auto global = get_global< my_global_t >();
        auto rng = global->get_random();

        x = rng->uniform_range( -10.0, 10.0 );
        y = rng->uniform_range( -10.0, 10.0 );

        if ( auto p = get_process() )
        {
            if ( auto id_opt = p->get_id() )
            {
                size_t id = id_opt.value();
                // Scrive nel buffer globale per inizializzare lo stato al t=0
                global->pos_current[id] = { x, y };
                global->pos_next[id] = { x, y };
            }
        }

        set_thread_time( 0.0 );
        set_compute_time( 0.0 );
        set_sleep_time( 0.0 );
    }

    void fun() override
    {
        auto global = get_global< my_global_t >();
        auto process = get_process();
        size_t id = process->get_id().value();

        // Lettura stato sincronizzato
        x = global->pos_current[id].first;
        y = global->pos_current[id].second;

        double theta = 0.0;
        auto rng = global->get_random();

        if ( id == 0 )
        { // Veicolo 1: Policy
            std::vector< int > counts( 4, 0 );
            double R_sq = global->radius * global->radius;
            double x1 = x;
            double y1 = y;

            for ( size_t i = 1; i < global->n_vehicles; ++i )
            {
                double xi = global->pos_current[i].first;
                double yi = global->pos_current[i].second;
                double dx = xi - x1;
                double dy = yi - y1;

                if ( dx * dx + dy * dy <= R_sq )
                {
                    if ( dx >= 0 && dy >= 0 )
                        counts[0]++;
                    else if ( dx >= 0 && dy <= 0 )
                        counts[1]++;
                    else if ( dx <= 0 && dy <= 0 )
                        counts[2]++;
                    else if ( dx <= 0 && dy >= 0 )
                        counts[3]++;
                }
            }

            int min_count = *std::min_element( counts.begin(), counts.end() );
            std::vector< int > best_quadrants;
            for ( int k = 0; k < 4; ++k )
            {
                if ( counts[k] == min_count )
                    best_quadrants.push_back( k );
            }

            int chosen_k = best_quadrants[rng->uniform_range( 0, (int) best_quadrants.size() - 1 )];
            double min_theta = chosen_k * ( M_PI / 2.0 );
            double max_theta = ( chosen_k + 1 ) * ( M_PI / 2.0 );
            theta = rng->uniform_range( min_theta, max_theta );
        }
        else
        {
            // Altri veicoli
            theta = rng->uniform_range( 0.0, 2.0 * M_PI );
        }

        // Calcolo next step
        double dist = global->time_step * global->velocity;
        double new_x = x + dist * std::sin( theta );
        double new_y = y + dist * std::cos( theta );

        global->pos_next[id] = { new_x, new_y };

        set_compute_time( 0.0 );
        set_sleep_time( global->time_step );
    }
};

class my_system_t : public system_t
{
public:
    using system_t::system_t;

    void init() override
    {
        system_t::init(); // Chiama thread::init -> posiziona veicoli

        auto global = get_global< my_global_t >();

        // Calcolo stato collisioni iniziale (t=0)
        double R_sq = global->radius * global->radius;
        double x1 = global->pos_current[0].first;
        double y1 = global->pos_current[0].second;

        global->current_run_collisions = 0;

        for ( size_t i = 1; i < global->n_vehicles; ++i )
        {
            double dx = global->pos_current[i].first - x1;
            double dy = global->pos_current[i].second - y1;
            global->collision_state[i] = ( dx * dx + dy * dy <= R_sq );
        }
    }

    void on_end_step() override
    {
        auto global = get_global< my_global_t >();

        // Swap buffer
        global->pos_current = global->pos_next;

        // Rilevamento transizione collisioni
        double R_sq = global->radius * global->radius;
        double x1 = global->pos_current[0].first;
        double y1 = global->pos_current[0].second;

        for ( size_t i = 1; i < global->n_vehicles; ++i )
        {
            double dx = global->pos_current[i].first - x1;
            double dy = global->pos_current[i].second - y1;
            bool is_in_radius = ( dx * dx + dy * dy <= R_sq );

            if ( is_in_radius && !global->collision_state[i] )
            {
                global->current_run_collisions++;
            }
            global->collision_state[i] = is_in_radius;
        }
    }
};

class my_simulator_t : public simulator_t
{
public:
    using simulator_t::simulator_t;

    bool should_terminate() override
    {
        auto global = get_global< my_global_t >();
        return get_system()->get_current_time() > global->horizon;
    };

    void on_terminate() override
    {
        auto global = get_global< my_global_t >();
        global->set_montecarlo_current( static_cast< double >( global->current_run_collisions ) );
    }
};

int main()
{
    auto global = std::make_shared< my_global_t >();
    auto writer = output_writer_t( "results.txt" );
    writer.write_line( "2025-01-09-AntonioMario-RossiPatrizio-1234567" );

    my_input_parser_t parser( global );
    parser.parse();

    global->set_montecarlo_budget( 1000 );

    auto system = std::make_shared< my_system_t >( global, "uv_system" );

    for ( size_t i = 0; i < global->n_vehicles; ++i )
    {
        auto process = process_t::create( "vehicle_" + std::to_string( i ) );
        process->set_id( i );
        process->add_thread( std::make_shared< my_thread_t >() );
        system->add_process( process );
    }

    auto simulator = std::make_shared< my_simulator_t >( system );
    auto montecarlo = montecarlo_t::create( simulator );

    montecarlo->run();

    double avg_collisions = global->get_montecarlo_avg();
    double C = 0.0;

    if ( avg_collisions > 1e-9 )
    {
        C = global->horizon / avg_collisions;
    }
    else
    {
        C = global->horizon;
    }

    writer.get_stream() << "C " << C << std::endl;

    return 0;
}
