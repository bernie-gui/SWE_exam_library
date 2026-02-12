#include <algorithm>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <unordered_map>
#include <algorithm>
#include <vector>
#include "global.hpp"
#include "io/lambda_parser.hpp"
#include "montecarlo.hpp"
#include "optimizer.hpp"
#include "process.hpp"
#include "simulator.hpp"
#include "system.hpp"
#include "utils/rate.hpp"
#include "customer-server/request.hpp"
#include "customer-server/server.hpp"
using namespace isw;

class requests_global : public global_t {
    public:
        size_t Q, P, S, C;
        double A, B, W, V, a, b;
        utils::rate_meas_t measure;
        void init() override {
            global_t::init();
            measure.init();
        }
};

class cust : public process_t {
    public:
        std::unordered_multimap<std::pair<size_t, size_t>, size_t> request_history;
        void init() override {
            process_t::init();
            request_history.clear();
        }
};

class cust_thread_1 : public thread_t {
    public:
        void fun() override {
            size_t server;
            cs::request_t msg;
            auto gl = get_global< requests_global >();
            auto p = get_process< cust >();
            msg.item = gl->get_random()->uniform_range(0, gl->P-1);
            msg.quantity = gl->get_random()->uniform_range(1, gl->Q);
            server = gl->get_random()->uniform_range(0, gl->S-1);
            p->request_history.emplace(std::make_pair(server, msg.item), msg.quantity);
            send_message("Servers", server, msg);
            set_compute_time(gl->get_random()->uniform_range(gl->A, gl->B));
        }
        void init() override {
            thread_t::init();
            auto gl = get_global< requests_global >();
            set_compute_time(gl->get_random()->uniform_range(gl->A, gl->B));
        }
};

class cust_thread_2 : public thread_t {
    public:
        void fun() override {
            std::shared_ptr<cs::request_t> msg;
            auto p = get_process<cust>();
            auto gl = get_global<requests_global>();
            // size_t quant;
            if ((msg = receive_message<cs::request_t>()) == nullptr) return;
            // quant = msg->quant;
            auto i = p->request_history.find({msg->sender_rel, msg->item});
            if (i != p->request_history.end()) {
                // if (quant < i->second) {
                //     gl->measure.update(1, get_thread_time());
                // }
                p->request_history.erase(i);
            }
            else throw std::runtime_error(" unknown sender ");
        }
        cust_thread_2() : thread_t(1, 0, 1) {}
};

class supplier_thread : public thread_t {
    public:
        void fun() override {
            auto gl = get_global<requests_global>();
            size_t server;
            cs::request_t msg;
            msg.item = gl->get_random()->uniform_range(0, gl->P-1);
            msg.quantity = gl->get_random()->uniform_range(1, gl->Q);
            server = gl->get_random()->uniform_range(0, gl->S-1);
            send_message("Servers", server, msg);
            set_compute_time(gl->get_random()->uniform_range(gl->V, gl->W));
        }
        void init() override {
            thread_t::init();
            auto gl = get_global<requests_global>();
            set_compute_time(gl->get_random()->uniform_range(gl->V, gl->W));
        }
};

class sim_help : public simulator_t {
    public:
        void on_terminate() override {
            auto gl = get_global<requests_global>();
            gl->set_montecarlo_current(gl->measure.get_rate());
        }
        sim_help(std::shared_ptr<system_t> s) : simulator_t(s) {}
};

class my_opt : public optimizer_t<int> {
    public:
        double obj_fun( std::vector<int> &arguments ) override {
            int F = arguments[0];
            size_t i;
            auto gl = get_global<requests_global>();
            auto sys = std::make_shared<system_t>(gl);
            for (i = 0; i < gl->S; i++ ) {
                sys->add_process( cs::server_t::create_process< cs::request_t >(
                    gl->P,
                    [gl](auto){return gl->get_random()->uniform_range(0, gl->Q);},
                    0.1,
                    {{"Customers", [gl](auto pt, auto msg){
                        auto p = pt->template get_process<cs::server_t>();
                        auto copy = std::min(p->database[msg->item], msg->quantity);
                        cs::request_t ret;
                        if (copy < msg->quantity) gl->measure.update(1, pt->get_thread_time());
                        p->database[msg->item] -= copy;
                        ret.quantity = copy;
                        ret.item = msg->item;
                        pt->send_message(msg->sender, ret);
                    }},
                    {"Suppliers", [](auto p, auto msg){
                        auto pt = p->template get_process<cs::server_t>();
                        pt->database[msg->item]+=msg->quantity;
                    }}},
                    0.2
                ), "Servers" );
            }
            for (i = 0; static_cast<int>(i) < F; i++) {
                sys->add_process( std::make_shared< process_t >()->add_thread( std::make_shared< supplier_thread >() ), "Suppliers" );
            }
            for (i = 0; i < gl->C; i++) {
                sys->add_process( std::make_shared< cust >()->add_thread( std::make_shared< cust_thread_1 >() )->add_thread(std::make_shared<cust_thread_2>()), "Customers" );
            }
            sys->add_network();
            auto sim = std::make_shared< sim_help >( sys );
            auto monty = montecarlo_t::create(sim);
            monty->run();
            return gl->a * F + gl->b * gl->get_montecarlo_avg();
        }
        my_opt(std::shared_ptr<requests_global> gl) : optimizer_t<int>(gl) {}
};

int main()
{
    auto gl = std::make_shared< requests_global >(); //must not be redone

    auto sys = std::make_shared< system_t >( gl);
    auto reader = lambda_parser_t("examples/example_01_26_4.txt", {
        {"A", [gl](auto& iss) { iss >> gl->A; }},
        {"V", [gl](auto& iss) { iss >> gl->V; }},
        {"C", [gl](auto& iss) { iss >> gl->C; }},
        {"B", [gl](auto& iss) { iss >> gl->B; }},
        {"G", [gl](auto& iss) { double temp; iss >> temp; gl->set_optimizer_budget(temp); }},
        {"a", [gl](auto& iss) { iss >> gl->a; }},
        {"b", [gl](auto& iss) { iss >> gl->b; }},
        {"W", [gl](auto& iss) { iss >> gl->W; }},
        {"P", [gl](auto& iss) { iss >> gl->P; }},
        {"Q", [gl](auto& iss) { iss >> gl->Q; }},
        {"S", [gl](auto& iss) { iss >> gl->S; }},
        {"H", [gl](auto& iss) { double temp; iss >> temp; gl->set_horizon(temp); }},
        {"M", [gl](auto& iss) { double temp; iss >> temp; gl->set_montecarlo_budget(temp); }}}
    ); //make markov parser
    reader.parse();
    auto opt = std::make_shared<my_opt>(gl);
    opt->optimize(optimizer_strategy::MINIMIZE, 1, 2*gl->S);
    std::cout << "Result: " << gl->get_optimizer_optimal_parameters()[0] << std::endl;
    std::cout << "Function: " << gl->get_optimizer_result() << std::endl;
    return 0;
}