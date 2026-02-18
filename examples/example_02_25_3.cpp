#include <cassert>
#include <cstddef>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <iostream>
#include "global.hpp"
#include "io/array_parser.hpp"
#include "montecarlo.hpp"
#include "network/message.hpp"
#include "process.hpp"
#include "simulator.hpp"
#include "system.hpp"
#include "utils/markov/markov.hpp"
#include "utils/rate.hpp"

using namespace isw;

class mkch_global : public global_t {
    public:
        markov::markov_chain_t clients;
        utils::rate_meas_t services;
        double T1, T2;

        mkch_global() : clients(3) {}
        void init() override {
            global_t::init();
            services.init();
        }
};

class markov_p : public process_t {
    public:
        size_t state;

        void init() override {
            process_t::init();  // !!!
            state = 0;
        }

        markov_p() : process_t("mogus"), state(0) {}
};

class cost : public network::message_t {
    public:
        double time;
        bool nothing;
};

class server_th : public thread_t {
    public:
        bool zero;
        double t;

        void fun() override {
            auto msg = receive_message< cost >();
            if (msg == nullptr) {
                set_compute_time(t);
                return;
            }
            auto gl = get_global< mkch_global >();
            if (!zero && msg->nothing) {
                gl->services.update(1, get_thread_time());
                zero = true;
                // std::cout << "succes " << zero << " " << msg->time << std::endl;
            }
            else if (zero) zero = false;
            set_compute_time(msg->time);
        }

        void init() override {
            thread_t::init();
            zero = true;
        }

        server_th(double t) : thread_t(t, 0, t), zero(true), t(t) {}
};

class markov_thread : public thread_t {
    public:

        markov_thread(double c_time) : thread_t(c_time, 0, c_time) {}
        
        void fun() override {
            auto gl = get_global<mkch_global>();
            auto p = get_process<markov_p>();
            cost msg;
            switch (p->state) {
                case 0: 
                    msg.time = 0;
                    msg.nothing = true;
                break;
                case 1:
                    msg.time = gl->T1;
                    msg.nothing = false;
                    // std::out << "oy" << std::endl;
                break;
                case 2: 
                    msg.time = gl->T2;
                    msg.nothing = false;
                break;
                default: throw std::runtime_error(" undefined state ");
            }
            // std::cout << msg.time << std::endl;
            // msg.time = 67;
            send_message("Servers", 0, msg);
            //p->cost += gl->clients.matrix[p->state][next].second; prob è il primo
            size_t next = gl->clients.next_state(p->state, gl->get_random()->get_engine());
            // std::cout << next << std::endl;
            p->state = next;
            // std::cout << p->state << std::endl;
        }
};

class mkch_sim : public simulator_t {
    public:
        // bool should_terminate() override {
        //     auto mk = get_system()->get_processes<markov_p>()[0];
        //     auto gl = get_global<mkch_global>();
        //     return mk->state == gl->diagram.matrix.size()-1;
        // }
        void on_terminate() override {
            auto mk = get_system()->get_processes<markov_p>()[0];
            auto gl = get_global<mkch_global>();
            gl->set_montecarlo_current(gl->services.get_rate());
        }
        mkch_sim(std::shared_ptr<system_t> sys) : simulator_t(sys) {}
};

int main() {
    auto gl = std::make_shared<mkch_global>();
    gl->set_horizon(10000);
    gl->set_montecarlo_budget(1000);

    auto load_prob = [gl](size_t from, size_t to) {
        return [gl, from, to](auto &iss) {
            size_t i, j;
            double prob;
            iss >> i >> j >> prob;
            assert(i == from);
            assert(j == to);
            gl->clients.matrix[from][to].first = prob;
        };
    };

    array_parser_t input(
        "examples/example_02_25_3.txt",
        {
            load_prob(0, 0),
            load_prob(0, 1),
            load_prob(0, 2),
            load_prob(1, 0),
            load_prob(1, 1),
            load_prob(1, 2),
            load_prob(2, 0),
            load_prob(2, 1),
            load_prob(2, 2),
            [gl](auto &iss){ iss >> gl->T1; },
            [gl](auto &iss){ iss >> gl->T2; }
        }
    );
    input.parse();
    auto sys = std::make_shared<system_t>(gl);
    auto sim = std::make_shared<mkch_sim>(sys);
    auto monty = montecarlo_t::create(sim);
    sys->add_process(std::make_shared<markov_p>()->add_thread(std::make_shared<markov_thread>(1)));
    sys->add_process(std::make_shared<process_t>()->add_thread(std::make_shared<server_th>(1)), "Servers");
    sys->add_network();
    monty->run();
    std::cout << "2025-01-09-Mario-Rossi-1234567" << std::endl 
        << "Avg " << gl->get_montecarlo_avg() << std::endl;
    // std::cout << "Cost: " << gl->get_montecarlo_avg() << std::endl;
    // for(auto &r : gl->diagram) {
    //     for(auto &c : r) {
    //         std::cout << "{" << c.first << "-" << c.second << "}" << " ";
    //     }
    //     std::cout << std::endl;
    // }
}