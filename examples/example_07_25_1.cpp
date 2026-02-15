#include <cassert>
#include <iostream>
#include <memory>
#include <cmath>
#include <vector>
#include "global.hpp"
#include "io/lambda_parser.hpp"
#include "montecarlo.hpp"
#include "simulator.hpp"
#include "system.hpp"
#include "process.hpp"
#include "utils/backtracking.hpp"
#include "utils/rate.hpp"

using namespace isw;

class my_global : public global_t {
    public:
        size_t N, Q;
        double A, T, V;
        utils::rate_meas_t measure;
    
        void init() override {
            global_t::init();
            measure.init();
        }
};

class my_vehicle : public process_t {
    public:
        double pos[2], targs[2];

        void init() override {
            process_t::init();
            auto gl = get_global();
            for (int i = 0; i < 2; i++) {
                pos[i] = gl->get_random()->uniform_range(-50.0, 50.0);
                targs[i] = -50 + 100 * static_cast<double>(i+1) / (get_relative_id().value()+1);
            }
        }        
};

class my_thread : public thread_t {
    public:
        void fun() override {
            auto gl = get_global< my_global >();
            auto p = get_process< my_vehicle >();
            double w[2];
            for (int i = 0; i < 2; i++) 
                w[i] = gl->get_random()->uniform_range(-2 * M_PI / gl->Q, 2 * M_PI / gl->Q);
            auto best = utils::arg_min_max<size_t, double>({std::make_pair(0, gl->Q-1)}, 
                [gl, p, w](auto pt){ size_t q = (*pt)[0];
                            auto th = (static_cast<double>(q)/gl->Q) * 2 * M_PI;
                            double newp[2];
                            newp[0] = p->pos[0] + gl->T * gl->V * sin(th + w[0]);
                            newp[1] = p->pos[1] + gl->T * gl->V * cos(th + w[1]);
                            double first = 0, second = 0;
                            for (int i = 0; i < 2; i++)
                                first += (newp[i] - p->targs[i]) * (newp[i] - p->targs[i]);
                            first *= gl->A;
                            for (auto j : p->get_system()->get_processes<my_vehicle>())
                                for (int i = 0; i < 2; i++) {
                                    if (j->get_relative_id() == p->get_relative_id()) continue;
                                    second += (j->pos[i] - p->pos[i]) * (j->pos[i] - p->pos[i]);
                            }
                            second *= (1 - gl->A);
                            // std::cout << first - second << std::endl;
                            return first - second;}, 
                utils::arg_strat::MIN );
            // std::cout << "end" << std::endl;
            auto one = utils::get_unif_random(best, gl->get_random()->get_engine());
            size_t q = (*one)[0];
            auto th = (static_cast<double>(q)/gl->Q) * 2 * M_PI;
            p->pos[0] = p->pos[0] + gl->T * gl->V * sin(th + w[0]);
            p->pos[1] = p->pos[1] + gl->T * gl->V * cos(th + w[1]);
        }

        my_thread(double time) : thread_t(time, 0, time) {}
};

class monitor_coll : public thread_t {
    public:
        void fun() override{
            std::vector< std::vector< bool > > collisions;
            size_t id1, id2, collision_count = 0;
            auto gl = get_global<my_global>();
            auto &ps = get_process()->get_system()->get_processes<my_vehicle>();
            double temp, dist;
            collisions.resize( gl->N );
            for ( auto &row : collisions )
                row.resize( gl->N );
            for ( auto v1 : ps )
                for ( auto v2 : ps ) {
                    id1 = v1->get_relative_id().value();
                    id2 = v2->get_relative_id().value();
                    if ( collisions[id2][id1] || id1 == id2 ) continue;
                    dist = 0;
                    for ( size_t i = 0; i < 2; i++ ) {
                        temp =  v1->pos[i] - v2->pos[i];
                        dist += temp * temp;
                    }
                    dist = std::sqrt( dist );
                    if (dist <= 0.1) {
                        collisions[id1][id2] = true;
                        collision_count++;
                    }
            }
            gl->measure.update(collision_count, get_thread_time());
        }

        monitor_coll(double time) : thread_t(time, 0, time) {}
};

class my_sim : public simulator_t {
    public:
        void on_terminate() override {
            auto gl = get_global<my_global>();
            gl->set_montecarlo_current(gl->measure.get_rate());
        }
        my_sim(std::shared_ptr<system_t> sys) : simulator_t(sys) {}
};

int main() {
    auto gl = std::make_shared<my_global>();
    lambda_parser_t parsy("examples/example_07_25_1.txt", {
        {"H", [gl](auto& iss) { double temp; iss >> temp; gl->set_horizon(temp); }},
        {"M", [gl](auto& iss) { double temp; iss >> temp; gl->set_montecarlo_budget(temp); }},
        {"N", [gl](auto &iss) { iss >> gl->N; }},
        {"A", [gl](auto &iss) { iss >> gl->A; }},
        {"Q", [gl](auto &iss) { iss >> gl->Q; }},
        {"T", [gl](auto &iss) { iss >> gl->T; }},
        {"V", [gl](auto &iss) { iss >> gl->V; }},
    });
    parsy.parse();
    auto sys = std::make_shared<system_t>(gl);
    for (size_t i = 0; i < gl->N; i++)
        sys->add_process(std::make_shared<my_vehicle>()->add_thread(std::make_shared<my_thread>(gl->T)));
    sys->add_process(std::make_shared<process_t>()->add_thread(std::make_shared<monitor_coll>(gl->T)));
    auto sim = std::make_shared<my_sim>(sys);
    auto monty = montecarlo_t::create(sim);
    monty->run();
    std::cout << "C " << gl->get_montecarlo_avg() << std::endl;
}