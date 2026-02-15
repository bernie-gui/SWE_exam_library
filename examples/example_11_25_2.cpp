#include <cstddef>
#include <memory>
#include <ostream>
#include <string>
#include <utility>
#include <vector>
#include <iostream>
#include "global.hpp"
#include "io/lambda_parser.hpp"
#include "io/output_writer.hpp"
#include "montecarlo.hpp"
#include "process.hpp"
#include "simulator.hpp"
#include "system.hpp"

using namespace isw;

class mkch_global : public global_t {
    public:
        std::vector<std::vector<std::pair<double, double>>> diagram;
        double c_max;

};

class markov_p : public process_t {
    public:
        size_t state;
        double cost;

        void init() override {
            process_t::init();  // !!!
            state = 0;
            cost = 0;
        }

        markov_p() : process_t("mogus"), state(0), cost(0) {}
};

class markov_thread : public thread_t {
    public:
        markov_thread(double c_time) : thread_t(c_time, 0, c_time) {}
        
        void fun() override {
            auto gl = get_global<mkch_global>();
            auto p = get_process<markov_p>();
            double extract = gl->get_random()->uniform_range(.0, 1.0);
            double accum = 0;
            for (size_t i = 0; i < gl->diagram.size(); i++) {
                accum += gl->diagram[p->state][i].first;
                if (extract <= accum) {
                    p->cost += gl->diagram[p->state][i].second;
                    p->state = i;
                    break;
                }
            }
        }
};

class mkch_sim : public simulator_t {
    public:
        bool should_terminate() override {
            auto mk = get_system()->get_processes<markov_p>()[0];
            auto gl = get_global<mkch_global>();
            return mk->state == gl->diagram.size()-1;
        }
        void on_terminate() override {
            auto mk = get_system()->get_processes<markov_p>()[0];
            auto gl = get_global<mkch_global>();
            gl->set_montecarlo_current(static_cast<double>(mk->cost) <= gl->c_max);
        }
        mkch_sim(std::shared_ptr<system_t> sys) : simulator_t(sys) {}
};

int main() {
    auto gl = std::make_shared<mkch_global>();
    lambda_parser_t input(
        "examples/example_11_25_2.txt",
        {{"M", [gl](auto &iss){ size_t temp; iss >> temp;  gl->set_montecarlo_budget(temp); }},
        {"N", [gl](auto &iss){ 
                size_t temp; iss >> temp; 
                gl->diagram.resize(temp);
                for (auto &el : gl->diagram) el.resize(temp);
            }},
        {"C", [gl](auto &iss){iss >> gl->c_max;}}}
    );
    input.parse();
    input.set_bindings({
        {"A", [gl](auto &iss){
            size_t i, j;
            double cost, prob;
            iss >> i >> j >> prob >> cost;
            gl->diagram[i][j] = {prob, cost};
        }}
    });
    input.reset_stream();
    input.parse();
    auto sys = std::make_shared<system_t>(gl);
    auto sim = std::make_shared<mkch_sim>(sys);
    auto monty = montecarlo_t::create(sim);
    sys->add_process(std::make_shared<markov_p>()->add_thread(std::make_shared<markov_thread>(1)));
    monty->run();
    output_writer_t out("examples/example_11_25_2out.txt");
    out << "2025-01-09-Mario-Rossi-1234567" << std::endl 
        << "P " << gl->get_montecarlo_avg() << std::endl;
}