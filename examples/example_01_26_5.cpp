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
#include "io/input_parser.hpp"
#include "montecarlo.hpp"
#include "network/message.hpp"
#include "optimizer.hpp"
#include "process.hpp"
#include "simulator.hpp"
#include "system.hpp"
#include "utils/rate.hpp"
using namespace isw;

class requests_global : public global_t {
    public:
        size_t Q, P, S, C;
        double A, B, V, W, a, b, F;
        utils::rate_meas_t measure;
        void init() override {
            global_t::init();
            measure.init();
        }
};

class sys_msg : public network::message_t {
    public:
        size_t item, quant;
};

class server : public process_t {
    public:
        std::vector<size_t> DB;
        void init() override {
            process_t::init();
            auto gl = get_global<requests_global>();
            DB.clear();
            for (size_t i = 0; i < gl->P; i++) 
                DB.emplace_back(gl->get_random()->uniform_range(0, gl->Q));
        }
};

class server_thread : public thread_t {
    public:
        void fun() override {
            std::shared_ptr< sys_msg > msg;
            sys_msg ret;
            size_t copy;
            auto p = get_process< server >();
            if ((msg = receive_message< sys_msg >()) == nullptr) return;
            if (msg->world_key == "Customers") {
                copy = std::min(p->DB[msg->item], msg->quant);
                p->DB[msg->item] -= copy;
                ret.quant = copy;
                ret.item = msg->item;
                send_message(msg->sender, ret);
            }
            else if (msg->world_key == "Suppliers") {
                p->DB[msg->item]+=msg->quant;
            }
            else throw std::runtime_error(" unknown sender type ");
        }

        server_thread() : thread_t(0.1, 0.2) {} // da rivedere sta cosa
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
            sys_msg msg;
            auto gl = get_global< requests_global >();
            auto p = get_process< cust >();
            msg.item = gl->get_random()->uniform_range(0, gl->P-1);
            msg.quant = gl->get_random()->uniform_range(1, gl->Q);
            server = gl->get_random()->uniform_range(0, gl->S-1);
            p->request_history.emplace(std::make_pair(server, msg.item), msg.quant);
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
            std::shared_ptr<sys_msg> msg;
            auto p = get_process<cust>();
            auto gl = get_global<requests_global>();
            size_t quant;
            if ((msg = receive_message<sys_msg>()) == nullptr) return;
            quant = msg->quant;
            auto i = p->request_history.find({msg->sender_rel, msg->item});
            if (i != p->request_history.end()) {
                if (quant < i->second) {
                    gl->measure.update(1, get_thread_time());
                }
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
            sys_msg msg;
            msg.item = gl->get_random()->uniform_range(0, gl->P-1);
            msg.quant = gl->get_random()->uniform_range(1, gl->Q);
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

class my_opt : public optimizer_t<double> {
    public:
        double obj_fun( std::vector<double> &arguments ) override {
            size_t i;
            auto gl = get_global<requests_global>();
            gl->V = arguments[0];
            gl->W = gl->V + 5;
            auto sys = std::make_shared<system_t>(gl);
            for (i = 0; i < gl->S; i++ ) {
                sys->add_process( std::make_shared< server >()->add_thread( std::make_shared< server_thread >() ), "Servers" );
            }
            for (i = 0; i < gl->F; i++) {
                sys->add_process( std::make_shared< process_t >()->add_thread( std::make_shared< supplier_thread >() ), "Suppliers" );
            }
            for (i = 0; i < gl->C; i++) {
                sys->add_process( std::make_shared< cust >()->add_thread( std::make_shared< cust_thread_1 >() )->add_thread(std::make_shared<cust_thread_2>()), "Customers" );
            }
            sys->add_network();
            auto sim = std::make_shared< sim_help >( sys );
            auto monty = montecarlo_t::create(sim);
            monty->run();
            return gl->a * gl->V + gl->b * gl->get_montecarlo_avg();
        }
        my_opt(std::shared_ptr<requests_global> gl) : optimizer_t<double>(gl) {}
};

int main()
{
    auto gl = std::make_shared< requests_global >(); //must not be redone

    auto sys = std::make_shared< system_t >( gl);
    auto reader = lambda_parser("examples/example_01_26_5.txt", {
        {"A", [gl](auto& iss) { iss >> gl->A; }},
        {"C", [gl](auto& iss) { iss >> gl->C; }},
        {"B", [gl](auto& iss) { iss >> gl->B; }},
        {"G", [gl](auto& iss) { double temp; iss >> temp; gl->set_optimizer_budget(temp); }},
        {"a", [gl](auto& iss) { iss >> gl->a; }},
        {"b", [gl](auto& iss) { iss >> gl->b; }},
        {"P", [gl](auto& iss) { iss >> gl->P; }},
        {"F", [gl](auto& iss) { iss >> gl->F; }},
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