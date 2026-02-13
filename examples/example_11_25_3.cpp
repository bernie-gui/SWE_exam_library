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
#include "optimizer.hpp"
#include "process.hpp"
#include "simulator.hpp"
#include "system.hpp"

using namespace isw;

class mkch_global : public global_t {
    public:
        std::vector<std::vector<std::pair<double, double>>> diagram;
        double K;
        std::shared_ptr<montecarlo_t> monty;
};

class markov : public process_t {
    public:
        size_t state;
        double cost;

        void init() override {
            process_t::init();  // !!!
            state = 0;
            cost = 0;
        }

        markov() : process_t("mogus"), state(0), cost(0) {}
};

class markov_thread : public thread_t {
    public:
        markov_thread(double c_time) : thread_t(c_time, 0, c_time) {}
        
        void fun() override {
            auto gl = get_global<mkch_global>();
            auto p = get_process<markov>();
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
            auto mk = get_system()->get_processes<markov>()[0];
            auto gl = get_global<mkch_global>();
            return mk->state == gl->diagram.size()-1;
        }
        void on_terminate() override {
            auto mk = get_system()->get_processes<markov>()[0];
            auto gl = get_global();
            gl->set_montecarlo_current(mk->cost);
        }
        mkch_sim(std::shared_ptr<system_t> sys) : simulator_t(sys) {}
};

class mkch_opt : public optimizer_t<double> {
    public:
        double obj_fun( std::vector<double> &arguments) override {
            auto p = arguments[0];
            auto gl = get_global<mkch_global>();
            auto temp = gl->diagram[0][0];
            temp.first = p;
            gl->diagram[0][0] = temp;
            temp = gl->diagram[0][1];
            temp.first = 1-p;
            gl->diagram[0][1] = temp;
            gl->monty->run();
            return gl->K * (1-p) + gl->get_montecarlo_avg();
        }

        mkch_opt(std::shared_ptr<mkch_global> gl) : optimizer_t<double>(gl) {} ///ricordare init -> super init!!!
};

int main() {
    auto gl = std::make_shared<mkch_global>();
    lambda_parser_t input(
        "examples/example_11_25_3.txt",
        {{"M", [gl](auto &iss){ size_t temp; iss >> temp;  gl->set_montecarlo_budget(temp); }},
        {"N", [gl](auto &iss){ 
                size_t temp; iss >> temp; 
                gl->diagram.resize(temp);
                for (auto &el : gl->diagram) el.resize(temp);
            }},
        {"B", [gl](auto &iss){size_t temp; iss >> temp; gl->set_optimizer_budget(temp); }},
        {"K", [gl](auto &iss){iss >> gl->K;}}}
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
    gl->monty = monty;
    sys->add_process(std::make_shared<markov>()->add_thread(std::make_shared<markov_thread>(1)));
    mkch_opt opty(gl);
    opty.optimize(optimizer_strategy::MINIMIZE, 0, 1);
    output_writer_t out("examples/example_11_25_3out.txt");
    out << "2025-01-09-Mario-Rossi-1234567" << std::endl 
        << "p " << gl->get_optimizer_optimal_parameters()[0] << std::endl
        << "C " << gl->get_optimizer_result() - gl->K * (1-gl->get_optimizer_optimal_parameters()[0]) << std::endl
        << "K " << gl->get_optimizer_result() << std::endl;
}