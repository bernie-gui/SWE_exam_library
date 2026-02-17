#include <cassert>
#include <iostream>
#include <memory>
#include <cmath>
#include <ostream>
#include <vector>
#include "global.hpp"
#include "io/array_parser.hpp"
#include "optimizer.hpp"
#include "simulator.hpp"
#include "system.hpp"
#include "process.hpp"
#include "utils/rate.hpp"

using namespace isw;

class my_global : public global_t {
    public:
        size_t N, Q;
        // double frac;
        double A, R, P1[3], P2[3];
        utils::rate_meas_t measure;
        std::shared_ptr<simulator_t> sim;
    
        void init() override {
            global_t::init();
            measure.init();
        }
};

class my_vehicle : public process_t {
    public:
        double pos[3];
        void init() override {
            process_t::init();

            auto gl = get_global< my_global >();
            for (int i = 0; i < 3; i++) {
                pos[i] = gl->get_random()->uniform_range(gl->P1[i], gl->P2[i]);
            }                        
        }        
};

class my_thread : public thread_t {
    public:
        void fun() override {
            auto gl = get_global< my_global >();
            auto p = get_process< my_vehicle >();
            double vel;
            for (int i = 0; i < 3; i++) {
                vel = gl->get_random()->uniform_range(-gl->A, gl->A);
                p->pos[i] = std::min(gl->P2[i], std::max(gl->P1[i], p->pos[i] + vel * get_compute_time()));
            }
        }

        my_thread(double time) : thread_t(time, 0, time) {}
};


class my_sys : public system_t {
    public:
        void on_end_step() override {
            size_t id1, id2;
            auto gl = get_global<my_global>();
            auto &ps = get_processes<my_vehicle>();
            double temp, dist;
            for ( auto v1 : ps ) {
                if (!v1->is_active()) continue;
                for ( auto v2 : ps ) {
                    if (!v2->is_active()) continue;
                    id1 = v1->get_relative_id().value();
                    id2 = v2->get_relative_id().value();
                    if ( id1 >= id2 ) continue;
                    dist = 0;
                    for ( size_t i = 0; i < 3; i++ ) {
                        temp =  v1->pos[i] - v2->pos[i];
                        dist += temp * temp;
                    }
                    dist = std::sqrt( dist );
                    if (dist <= gl->R) {
                        v1->set_active(false);
                        v2->set_active(false);
                    }
                }
            }
        }

        my_sys(std::shared_ptr<my_global> gl) : system_t(gl) {}
};

class my_sim : public simulator_t {
    public:
        void on_terminate() override {
            auto gl = get_global<my_global>();
            auto &prs = get_system()->get_processes<my_vehicle>();
            gl->Q = 0;
            for ( auto p : prs ) {
                if (p->is_active()) {
                    gl->Q++;
                } 
            }
            // gl->frac = (double)gl->Q / (double)gl->N;
        }
        my_sim(std::shared_ptr<system_t> sys) : simulator_t(sys) {}
};

class my_opt : public optimizer_t<double> {
    public:
        double obj_fun(std::vector<double> &args) override {
            auto gl = get_global< my_global >();
            gl->A = args[0];
            gl->sim->run();
            return  static_cast<double>(gl->Q) / gl->N;
        }

        my_opt(std::shared_ptr<my_global> gl) : optimizer_t<double>(gl) {}
};

int main() {
    auto gl = std::make_shared<my_global>();

    array_parser_t parsy("examples/example_03_25_2.txt", {
        [&gl](auto& iss) { char ds; double temp; iss >> ds; iss >> temp; gl->set_horizon(temp); },
        [&gl](auto& iss) { char ds; iss >> ds; iss >> gl-> N; },
        // [&gl](auto& iss) { char ds; iss >> ds; iss >> gl-> A; },
        [&gl](auto& iss) { char ds; iss >> ds; iss >> gl-> R; },
        [&gl](auto& iss) { iss >> gl->P1[0]; iss >> gl->P2[0]; iss >> gl->P1[1]; iss >> gl->P2[1]; iss >> gl->P1[2]; iss >> gl->P2[2]; },
    }); 
    
    // lambda_parser_t parsy("examples/example_07_25_1.txt", {
    //     {"H", [gl](auto& iss) { double temp; iss >> temp; gl->set_horizon(temp); }},
    //     {"M", [gl](auto& iss) { double temp; iss >> temp; gl->set_montecarlo_budget(temp); }},
    //     {"N", [gl](auto &iss) { iss >> gl->N; }},
    //     {"A", [gl](auto &iss) { iss >> gl->A; }},
    //     {"Q", [gl](auto &iss) { iss >> gl->Q; }},
    //     {"T", [gl](auto &iss) { iss >> gl->T; }},
    //     {"V", [gl](auto &iss) { iss >> gl->V; }},
    // });D
    parsy.parse();
    auto sys = std::make_shared<my_sys>(gl);
    for (size_t i = 0; i < gl->N; i++)
        sys->add_process(std::make_shared<my_vehicle>()->add_thread(std::make_shared<my_thread>(1)));
    // sys->add_process(std::make_shared<process_t>()->add_thread(std::make_shared<monitor_coll>(1)));
    gl->sim = std::make_shared<my_sim>(sys);
    // gl->monty = montecarlo_t::create(sim);
    // monty->run();
    // sim->run();
    gl->set_optimizer_budget(1000);
    my_opt opty(gl);
    opty.optimize(optimizer_strategy::MAXIMIZE, 0.1, 0.5);
    std::cout << "2025-03-25-Pellarin" << std::endl;
    std::cout << "P " << gl->get_optimizer_result() << std::endl
        << "A " << gl->get_optimizer_optimal_parameters()[0] << std::endl;
}