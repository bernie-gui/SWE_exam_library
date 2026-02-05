#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include "io/input_parser.hpp"
#include "process.hpp"
#include "simulator.hpp"
#include "system.hpp"
using namespace isw;

class uav_global : public global_t
{
public:
    double A, L, T, V, R, D, last_collsion = 0, rate = 0;
};

class uav : public process_t
{
public:
    double pos[3], vel[3];
    uav() : process_t( "uav" ) {}

    void init() override
    {
        process_t::init();
        auto gl = get_global< uav_global >();
        for ( int i = 0; i < 3; i++ )
        {
            pos[i] = gl->get_random()->uniform_range( static_cast< double >( -gl->L ), static_cast< double >( gl->L ) );
            vel[i] = 0;
        }
    }
};

class uav_thread : public thread_t
{
private:
    static double prob( int k, std::shared_ptr< uav > p, std::shared_ptr< uav_global > gl )
    {
        return std::exp( -gl->A * ( p->pos[k] + gl->L ) / 2 * gl->L );
    }

public:
    void fun() override
    {
        auto gl = get_global< uav_global >();
        auto p = get_process< uav >();
        double pb;
        for ( int i = 0; i < 3; i++ )
        {
            p->pos[i] += p->vel[i] * gl->T;
            pb = prob( i, p, gl );
            p->vel[i] = gl->get_random()->uniform_range( 0.0, 1.0 ) < pb ? gl->V : -gl->V;
        }
    }
    void init() override
    {
        thread_t::init();
        auto gl = get_global< uav_global >();
        set_compute_time( gl->T );
    }
};

class coll_det_thread : public thread_t
{
public:
    void fun() override
    {
        auto &proc_list = get_process()->get_system()->get_processes< uav >( "UAVs" );
        auto gl = get_global< uav_global >();
        std::vector< std::vector< bool > > collisions;
        size_t id1, id2, collsion_count = 0;
        double dist = 0;
        collisions.resize( proc_list.size() );
        for ( auto &row : collisions )
            row.resize( proc_list.size() );
        for ( auto &p1 : proc_list )
            for ( auto &p2 : proc_list )
            {
                id1 = p1->get_relative_id().value();
                id2 = p2->get_relative_id().value();
                if ( collisions[id2][id1] || id1 == id2 )
                    continue;
                dist = 0;
                for ( int i = 0; i < 3; i++ )
                {
                    dist += ( p1->pos[i] - p2->pos[i] ) * ( p1->pos[i] - p2->pos[i] );
                }
                dist = std::sqrt( dist );
                if ( dist > gl->D )
                    continue;
                // std::cout << "HIT!!! Between " << id1 << " and " << id2 << ", Time is " << get_thread_time() << std::endl;
                collisions[id1][id2] = true;
                collsion_count++;
            }
        gl->rate = gl->rate * ( gl->last_collsion / get_thread_time() ) + collsion_count / get_thread_time();
        gl->last_collsion = get_thread_time();
    }
    void init() override
    {
        thread_t::init();
        auto gl = get_global< uav_global >();
        set_thread_time( gl->R );
        set_compute_time( gl->R );
    }
};

int main()
{
    auto gl = std::make_shared< uav_global >();
    auto sys = std::make_shared< system_t >( gl);
    size_t N;
    auto reader = lambda_parser("bins/example_01_26_1.txt", {
        {"A", [gl](auto& iss) { iss >> gl->A; }},
        {"L", [gl](auto& iss) { iss >> gl->L; }},
        {"V", [gl](auto& iss) { iss >> gl->V; }},
        {"R", [gl](auto& iss) { iss >> gl->R; }},
        {"D", [gl](auto& iss) { iss >> gl->D; }},
        {"T", [gl](auto& iss) { iss >> gl->T; }},
        {"H", [gl](auto& iss) { double temp; iss >> temp; gl->set_horizon(temp); }},
        {"N", [&N](auto& iss) { iss >> N; }}}
    );
    reader.parse();
    for ( size_t i = 0; i < N; i++ ) {
        sys->add_process( std::make_shared< uav >()->add_thread( std::make_shared< uav_thread >() ), "UAVs" );
    }
    sys->add_process(
        std::make_shared< process_t >( "collision_detector" )->add_thread( std::make_shared< coll_det_thread >() ),
        "monitors" );

    auto sim = std::make_shared< simulator_t >( sys );

    std::cout << "Starting simulation..." << std::endl;

    sim->run();

    auto final_gl = sys->get_global< uav_global >();
    std::cout << "Simulation ended. Collision rate: " << final_gl->rate << std::endl;

    return 0;
}
