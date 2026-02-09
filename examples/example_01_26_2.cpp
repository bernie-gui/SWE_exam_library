#include <cstddef>
#include <cstring>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "global.hpp"
#include "io/lambda_parser.hpp"
#include "montecarlo.hpp"
#include "process.hpp"
#include "simulator.hpp"
#include "system.hpp"
#include "utils/backtracking.hpp"
#include "utils/rate.hpp"
using namespace isw;

class uav_global : public global_t
{
public:
    double A, L, T, V, R, D, N;
    utils::rate_meas_t measure;
    void init() override {
        global_t::init();
        measure.init();
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
public:
    void fun() override //policy
    {
        auto gl = get_global< uav_global >();
        auto p = get_process< uav >();
        for ( int i = 0; i < 3; i++ )
            p->pos[i] += p->vel[i] * gl->T;
        std::unordered_set<double> x = { gl->V, -gl->V };
        std::function<double(std::shared_ptr<std::vector<double>>)> f = [this](auto v) {
            auto gl = this->get_global<uav_global>();
            auto procs = get_process()->get_system()->get_processes< uav >("UAVs");
            auto uavv = this->get_process<uav>();
            double add, ret = 0;
            for (auto &p : procs) {
                if (p->get_id() == get_process()->get_id()) continue;
                for (size_t k = 0; k < 3; k++) {
                    add = ((uavv->pos[k] + (*v)[k] * gl->T) - p->pos[k]) / (2 * gl->L);
                    add *= add;
                    ret += add;
                }
            }
            return ret;
        };
        auto pool = arg_min_max({x, x, x}, f, utils::arg_strat::MIN);
        auto it = utils::get_unif_random(pool, gl->get_random()->get_engine());
        for (size_t i = 0; i < 3; i++)
            p->vel[i] = (*it)[i];
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
        gl->measure.update(collsion_count, get_thread_time());
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
            gl->set_montecarlo_current(gl->measure.get_rate());
        }

        my_sim(std::shared_ptr<system_t> sys) : simulator_t(sys) {} 
};

int main()
{
    auto gl = std::make_shared< uav_global >();
    auto sys = std::make_shared< system_t >( gl);
    auto reader = lambda_parser_t("examples/example_01_26_1.txt", {
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
