#include <cstddef>
#include <cstring>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>
#include "global.hpp"
#include "io/input_parser.hpp"
#include "montecarlo.hpp"
#include "process.hpp"
#include "simulator.hpp"
#include "system.hpp"
using namespace isw;

class uav_global : public global_t
{
public:
    double A, L, T, V, R, D, N,
        last_collsion = 0, rate = 0;
    void init() override {
        global_t::init();
        last_collsion = 0;
        rate = 0;
    }
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
    inline double d(double *test) {
        auto gl = get_global< uav_global >();
        auto procs = get_process()->get_system()->get_processes< uav >("UAVs");
        double add, ret = 0;
        for (auto &p : procs) {
            if (p->get_id() == get_process()->get_id()) continue;
            for (size_t k = 0; k < 3; k++) {
                add = (test[k] - p->pos[k]) / (2 * gl->L);
                add *= add;
                ret += add;
            }
        }
        return ret;
    }
    void backtrack(double *vel2, double *pos, double t, 
            double *best, double *best_obj, size_t i = 0) { //generalize
        if (i == 3) { //terminal function
            double test[3];
            for (size_t j = 0; j < 3; j++) 
                test[j] = pos[j] + vel2[j] * t;
            double temp = d(test); //function call
            if (temp < *best) {
                *best = temp;
                memcpy(best_obj, vel2, 3 * sizeof(double));
            }
            return;
        }
        vel2[i] *= -1; //iterate over values
        backtrack(vel2, pos, t, best, best_obj, i+1);
        vel2[i] *= -1;
        backtrack(vel2, pos, t, best, best_obj, i+1);
    }

    inline void minimize(double *pos, double t, double *res) { //argmin (for discrete values) ritorna insieme!!!
        auto gl = get_global< uav_global >();
        double best = std::numeric_limits<double>::infinity();
        double vel2[] = {gl->V, gl->V, gl->V};
        backtrack(vel2, pos, t, &best, res);
    }

public:
    void fun() override //policy
    {
        auto gl = get_global< uav_global >();
        auto p = get_process< uav >();
        for ( int i = 0; i < 3; i++ )
            p->pos[i] += p->vel[i] * gl->T;
        minimize(p->pos, gl->T, p->vel);
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
        gl->rate = gl->rate * ( gl->last_collsion / get_thread_time() ) + collsion_count / get_thread_time(); //rate formula
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

class my_sim : public simulator_t {
    public:
        void on_terminate() override {
            auto gl = get_global<uav_global>();
            gl->set_montecarlo_current(gl->rate);
        }

        my_sim(std::shared_ptr<system_t> sys) : simulator_t(sys) {} 
};

int main()
{
    auto gl = std::make_shared< uav_global >();
    auto sys = std::make_shared< system_t >( gl);
    auto reader = lambda_parser("examples/example_01_26_1.txt", {
        {"A", [gl](auto& iss) { iss >> gl->A; }},
        {"L", [gl](auto& iss) { iss >> gl->L; }},
        {"V", [gl](auto& iss) { iss >> gl->V; }},
        {"R", [gl](auto& iss) { iss >> gl->R; }},
        {"D", [gl](auto& iss) { iss >> gl->D; }},
        {"T", [gl](auto& iss) { iss >> gl->T; }},
        {"H", [gl](auto& iss) { double temp; iss >> temp; gl->set_horizon(temp); }},
        {"N", [gl](auto& iss) { iss >> gl->N; }},
        {"M", [gl](auto& iss) { double temp; iss >> temp; gl->set_montecarlo_budget(temp); }}}
    );
    reader.parse();
    for ( size_t i = 0; i < gl->N; i++ ) {
        sys->add_process( std::make_shared< uav >()->add_thread( std::make_shared< uav_thread >() ), "UAVs" );
    }
    sys->add_process(
        std::make_shared< process_t >( "collision_detector" )->add_thread( std::make_shared< coll_det_thread >() ),
        "monitors" );

    auto sim = std::make_shared< my_sim >( sys );

    auto monty = montecarlo_t::create(sim);

    std::cout << "Starting simulation..." << std::endl;

    monty->run();

    std::cout << "Simulation ended. Collision rate: " << gl->get_montecarlo_avg() << std::endl;

    return 0;
}
