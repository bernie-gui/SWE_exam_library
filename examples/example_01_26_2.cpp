#include <cstddef>
#include <cstring>
#include <functional>
#include <iostream>
#include <memory>
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
#include "vehicles/vehicle.hpp"
#include "vehicles/functions.hpp"
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
class coll_det_thread : public thread_t // generalize (euclidean distance)
{
public:
    void fun() override
    {
        auto gl = get_global<uav_global>();
        auto &proc_list = get_process()->get_system()->get_processes< uv::vehicle_t >( "UAVs" );      
        gl->measure.update(isw::uv::count_collisions(proc_list, gl->D, 3), get_thread_time());
    }
    
    coll_det_thread(double c_time, double th_time) : thread_t(c_time, 0, th_time) {}
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
        sys->add_process( uv::vehicle_t::create_process(3, gl->T, 
            [gl](auto){return gl->get_random()->uniform_range( static_cast< double >( -gl->L ), static_cast< double >( gl->L )); }, 
            [](auto){return 0;}, 
            [gl](auto p){
                auto pt = p->template get_process< uv::vehicle_t >();
                for ( int i = 0; i < 3; i++ ) pt->pos[i] += pt->vel[i] * gl->T;
                std::unordered_set<double> x = { gl->V, -gl->V };
                std::function<double(std::shared_ptr<std::vector<double>>)> f = [gl, pt](auto v) {
                    auto procs = pt->get_system()->template get_processes< uv::vehicle_t >("UAVs");
                    double add, ret = 0;
                    for (auto &p : procs) {
                        if (p->get_id() == pt->get_id()) continue;
                        for (size_t k = 0; k < 3; k++) {
                            add = ((pt->pos[k] + (*v)[k] * gl->T) - p->pos[k]) / (2 * gl->L);
                            add *= add;
                            ret += add;
                        }
                    }
                    return ret;
                };
                auto pool = arg_min_max({x, x, x}, f, utils::arg_strat::MIN);
                auto it = utils::get_unif_random(pool, gl->get_random()->get_engine());
                for (size_t i = 0; i < 3; i++) pt->vel[i] = (*it)[i];
            }), "UAVs" );
    }
    sys->add_process(
        std::make_shared< process_t >( "collision_detector" )->add_thread( std::make_shared< coll_det_thread >(gl->R, gl->R) ),
        "monitors" );

    auto sim = std::make_shared< my_sim >( sys );

    auto monty = montecarlo_t::create(sim);

    std::cout << "Starting simulation..." << std::endl;

    monty->run();

    std::cout << "Simulation ended. Collision rate: " << gl->get_montecarlo_avg() << std::endl;

    return 0;
}
