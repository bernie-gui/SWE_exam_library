#include <cassert>
#include <iostream>
#include <memory>
#include <cmath>
#include <ostream>
#include "global.hpp"
#include "io/lambda_parser.hpp"
#include "io/output_writer.hpp"
#include "simulator.hpp"
#include "system.hpp"
#include "process.hpp"
#include "utils/rate.hpp"

using namespace isw;

class my_global : public global_t {
    public:
        size_t N;
        double T, V;
        utils::rate_meas_t measure;
        std::shared_ptr<output_writer_t> out;
    
        void init() override {
            global_t::init();
            measure.init();
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
            auto q = gl->get_random()->uniform_range(0, 3); 
            auto th = (q / 4.0) * 2 * M_PI;
            p->pos[0] = p->pos[0] + gl->T * gl->V * sin(th);
            p->pos[1] = p->pos[1] + gl->T * gl->V * cos(th);
        }

        my_thread(double time) : thread_t(time, 0, time) {}
};

class my_sys : public system_t {
    public:
        void on_end_step() override {
            auto &ps = get_processes< my_vehicle >();
            auto gl = get_global< my_global >();
            if (get_current_time() > gl->get_horizon()) return;
            for (auto p : ps) {
                (*gl->out) << get_current_time() << " "
                    << p->get_relative_id().value()+1 << " " << p->pos[0] << " "
                    << p->pos[1] << std::endl;
            }
        }
        my_sys(std::shared_ptr<my_global> gl) : system_t(gl) {}
};

// class my_sim : public simulator_t {
//     public:
//         void on_terminate() override {
//             auto gl = get_global<my_global>();
//             gl->set_montecarlo_current(gl->measure.get_rate());
//         }
//         my_sim(std::shared_ptr<system_t> sys) : simulator_t(sys) {}
// };

int main() {
    auto gl = std::make_shared<my_global>();
    lambda_parser_t parsy("examples/example_06_25_1.txt", {
        {"H", [gl](auto& iss) { double temp; iss >> temp; gl->set_horizon(temp); }},
        {"N", [gl](auto &iss) { iss >> gl->N; }},
        {"T", [gl](auto &iss) { iss >> gl->T; }},
        {"V", [gl](auto &iss) { iss >> gl->V; }},
    });
    parsy.parse();
    auto sys = std::make_shared<my_sys>(gl);
    for (size_t i = 0; i < gl->N; i++)
        sys->add_process(std::make_shared<my_vehicle>()->add_thread(std::make_shared<my_thread>(gl->T)));
    // sys->add_process(std::make_shared<process_t>()->add_thread(std::make_shared<monitor_coll>(gl->T)));
    auto sim = std::make_shared<simulator_t>(sys);
    // auto monty = montecarlo_t::create(sim);
    // monty->run();

    gl->out = std::make_shared<output_writer_t>("examples/example_06_25_1out.txt");

    (*gl->out) << "2025-01-09-Mario-Rossi-1234567" << std::endl;

    sim->run();
}