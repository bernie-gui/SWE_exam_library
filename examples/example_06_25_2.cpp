#include <cassert>
#include <cstddef>
#include <iostream>
#include <limits>
#include <memory>
#include <cmath>
#include <ostream>
#include "global.hpp"
#include "io/lambda_parser.hpp"
#include "io/output_writer.hpp"
#include "montecarlo.hpp"
#include "simulator.hpp"
#include "system.hpp"
#include "process.hpp"
#include "utils/rate.hpp"

using namespace isw;

class my_global : public global_t {
    public:
        size_t N;
        double T, V, R, last_collsion;
        utils::rate_meas_t measure;
        std::shared_ptr<output_writer_t> out;
    
        void init() override {
            global_t::init();
            measure.init();
            last_collsion = -1;
        }
};

class my_vehicle : public process_t {
    public:
        double pos[2];

        void init() override {
            process_t::init();
            auto gl = get_global();
            for (int i = 0; i < 2; i++) {
                pos[i] = gl->get_random()->uniform_range(-10.0, 10.0);
            }
        }        
};

class my_thread : public thread_t {
    public:
        void fun() override {
            auto gl = get_global< my_global >();
            auto p = get_process< my_vehicle >();
            auto ps = p->get_system()->get_processes< my_vehicle >();
            size_t quadrant[] = {0, 0, 0, 0}, min = 0, arg_min;
            for (auto pr : ps) {
                if (p->get_relative_id().value() == pr->get_relative_id().value()) continue;
                if (std::sqrt(std::pow(p->pos[0] - pr->pos[0], 2) + std::pow(p->pos[1] - pr->pos[1], 2)) <= gl->R) { //euclidean distance
                    if ((p->pos[0] - pr->pos[0]) >= 0 && 
                        (p->pos[1] - pr->pos[1]) >= 0) quadrant[0]++; // 0
                    else if ((p->pos[0] - pr->pos[0]) >= 0 && 
                        (p->pos[1] - pr->pos[1]) <= 0) quadrant[1]++;
                    else if ((p->pos[0] - pr->pos[0]) <= 0 && 
                        (p->pos[1] - pr->pos[1]) <= 0) quadrant[2]++;
                    else if ((p->pos[0] - pr->pos[0]) <= 0 && 
                        (p->pos[1] - pr->pos[1]) >= 0) quadrant[3]++;
                }
            }
            for (int i = 0; i < 4; i++) {
                if (quadrant[i] < min) {
                    arg_min = i;
                    min = quadrant[i];
                }
            }
            double th = gl->get_random()->uniform_range(arg_min * M_PI / 2, 
                                                        (arg_min + 1) * M_PI / 2);
            p->pos[0] = p->pos[0] + gl->T * gl->V * sin(th);
            p->pos[1] = p->pos[1] + gl->T * gl->V * cos(th);
        }

        my_thread(double time) : thread_t(time, 0, time) {}
};

class my_sys : public system_t {
    public:
        void on_end_step() override {
            size_t id;
            auto gl = get_global<my_global>();
            auto &ps = get_processes<my_vehicle>();
            double temp, dist;
            for ( auto v : ps ) {
                id = v->get_relative_id().value();
                if ( id == 0) continue; // count each unordered pair only once
                dist = 0;
                for ( size_t i = 0; i < 2; i++ ) {
                    temp =  ps[0]->pos[i] - v->pos[i];
                    dist += temp * temp;
                }
                dist = std::sqrt( dist );
                if (dist <= gl->R) {
                    if (gl->last_collsion != -1) {
                        gl->measure.increase_denom(1);
                        gl->measure.increase_amount(get_current_time() - gl->last_collsion);
                    }
                    gl->last_collsion = get_current_time();
                    break;
                }
            }
        }
        my_sys(std::shared_ptr<my_global> gl) : system_t(gl) {}
};

class my_sim : public simulator_t {
    public:
        void on_terminate() override {
            auto gl = get_global<my_global>();
            gl->set_montecarlo_current(gl->measure.was_updated() ? gl->measure.get_rate() : std::numeric_limits<double>::infinity());
        }
        my_sim(std::shared_ptr<system_t> sys) : simulator_t(sys) {}
};

int main() {
    auto gl = std::make_shared<my_global>();
    lambda_parser_t parsy("examples/example_06_25_1.txt", {
        {"H", [gl](auto& iss) { double temp; iss >> temp; gl->set_horizon(temp); }},
        {"N", [gl](auto &iss) { iss >> gl->N; }},
        {"T", [gl](auto &iss) { iss >> gl->T; }},
        {"V", [gl](auto &iss) { iss >> gl->V; }},
        {"R", [gl](auto &iss) { iss >> gl->R; }}
    });
    parsy.parse();
    auto sys = std::make_shared<my_sys>(gl);
    for (size_t i = 0; i < gl->N; i++)
        sys->add_process(std::make_shared<my_vehicle>()->add_thread(std::make_shared<my_thread>(gl->T)));
    // sys->add_process(std::make_shared<process_t>()->add_thread(std::make_shared<monitor_coll>(gl->T)));
    auto sim = std::make_shared<my_sim>(sys);
    // auto monty = montecarlo_t::create(sim);
    // monty->run();

    auto monty = montecarlo_t::create(sim);
    gl->set_montecarlo_budget(1000);
    monty->run();
    std::cout << "C " << gl->get_montecarlo_avg() << std::endl;
}