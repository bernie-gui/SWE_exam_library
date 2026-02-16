#include <cassert>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <memory>
// #include <tuple>
#include <unordered_map>
#include <vector>
// #include "utils/customer-server/utils.hpp"
// #include "utils/customer-server/server.hpp"
#include "global.hpp"
#include "io/lambda_parser.hpp"
#include "montecarlo.hpp"
#include "network/message.hpp"
#include "optimizer.hpp"
#include "process.hpp"
#include "simulator.hpp"
#include "system.hpp"
#include "utils/customer-server/server.hpp"
#include "utils/rate.hpp"
using namespace isw;

class requests_global : public global_t {
    public:
        size_t S, N, K;
        double A, B, T, D, p, F;
        utils::rate_meas_t q;
        std::shared_ptr<montecarlo_t> monty;
        void init() override {
            global_t::init();
            q.init();
            // measure_v.init();
            // measure_w.init();
        }
};


class my_msg : public network::message_t {
    public:
        size_t server, item;
        bool success;
};


class cust_thread_1 : public thread_t {
    public:
        void fun() override {
            // size_t server;
            // cs::request_t msg;
            auto gl = get_global< requests_global >();
            my_msg send;
            send.item = gl->get_random()->uniform_range(0, gl->N);
            send_message("Dispatchers", 0, send);
            set_compute_time(gl->get_random()->uniform_range(gl->A, gl->B));
        }
        void init() override {
            thread_t::init();
            auto gl = get_global< requests_global >();
            set_compute_time(gl->get_random()->uniform_range(gl->A, gl->B));
        }
};

// class my_sys : public system_t {
//     public:
//         void on_end_step() override {
//             auto &ps = get_processes();
//             // for (auto p : ps) {
//             //     std::cout << p->get_world_key().value() << " - " 
//             //         << p->get_relative_id().value() << ": " << p->next_update_time() << std::endl;
//             // }
//         }

//         my_sys(std::shared_ptr<requests_global> gl) : system_t(gl) {}
// };

class sim_help : public simulator_t {
    public:
        void on_terminate() override {
            auto gl = get_global<requests_global>();
            // gl->measure.update(0, gl->get_horizon()); //tecnicamente corretto ma cambia poco
            gl->set_montecarlo_current(gl->q.get_rate());
            // gl->set_montecarlo_current(gl->measure_w.get_rate(), 1);
        }
        sim_help(std::shared_ptr<system_t> s) : simulator_t(s) {}
};

class my_opt : public optimizer_t<double> {
    public:
        double obj_fun(std::vector<double> &args) override {
            auto p = args[0];
            auto gl = get_global<requests_global>();
            gl->p = p;
            gl->monty->run();
            return gl->get_montecarlo_avg();
        }

        my_opt(std::shared_ptr<requests_global> gl) : optimizer_t<double>(gl) {}
};

int main()
{
    auto gl = std::make_shared< requests_global >();
    auto sys = std::make_shared< system_t >( gl);
    auto reader = lambda_parser_t("examples/example_06_25_5.txt", {
        {"A", [gl](auto& iss) { iss >> gl->A; }},
        {"B", [gl](auto& iss) { iss >> gl->B; }},
        {"S", [gl](auto& iss) { iss >> gl->S; }},
        {"T", [gl](auto& iss) { iss >> gl->T; }},
        {"K", [gl](auto& iss) { iss >> gl->K; }},
        {"F", [gl](auto& iss) { iss >> gl->F; }},
        {"W", [gl](auto& iss) { double temp; iss >> temp; gl->set_optimizer_budget(temp); }},
        {"D", [gl](auto& iss) { iss >> gl->D; }},
        {"N", [gl](auto& iss) { iss >> gl->N; }},
        {"H", [gl](auto& iss) { double temp; iss >> temp; gl->set_horizon(temp); }},
        {"M", [gl](auto& iss) { double temp; iss >> temp; gl->set_montecarlo_budget(temp); }}
    });
    reader.parse();
    for (size_t i = 0; i < gl->S; i++)
        sys->add_process( cs::server_t::create_process< my_msg >(
            gl->N,
            [gl](auto){return gl->get_random()->uniform_range(1, gl->K);},
            gl->F,
            {{"Dispatchers", [gl](auto pt, auto msg){
                auto p = pt->template get_process<cs::server_t>();
                my_msg ret;
                if (p->database[msg->item] == 0) ret.success = false;
                else {
                    ret.success = true;
                    p->database[msg->item]--;
                }
                ret.item = msg->item;
                ret.server = msg->server;
                assert(ret.server == pt->get_process()->get_relative_id().value());
                // ret.tag = msg->tag;
                pt->send_message("Dispatchers", 0, ret);
            }}}
        ), "Servers" );
    sys->add_process( std::make_shared< process_t >()->add_thread( std::make_shared< cust_thread_1 >() ), "Customers" );
    sys->add_process(cs::server_t::create_process< my_msg >(
        0,
        [](auto){ return 0; },
        gl->D,
        {
            {"Customers", [gl](auto th, auto msg) { 
                my_msg msg2;
                if (msg->item == 0) return;
                msg2.item = msg->item-1;
                msg2.server = gl->get_random()->uniform_range(0, gl->S-1);
                th->send_message("Servers", msg2.server, msg2);
                gl->q.increase_denom(1);
            }},
            {"Servers", [gl](auto th, auto msg) {
                if (msg->success) return;
                auto &servers = th->get_process()->get_system()->template get_processes<cs::server_t>("Servers");
                std::shared_ptr<cs::server_t> one;
                gl->q.increase_amount(1);
                auto extract = gl->get_random()->uniform_range(0.0, 1.0);
                if (extract < gl->p) {
                    one = servers[msg->server];
                    one->database[msg->item]+=10;
                }
                else {
                    one = servers[gl->get_random()->uniform_range(0, gl->S-1)];
                    one->database[gl->get_random()->uniform_range(0, gl->N-1)]+=10;
                }
            }}}), "Dispatchers");
    sys->add_network();

    auto sim = std::make_shared< sim_help >( sys );    

    gl->monty = montecarlo_t::create(sim);

    my_opt opty(gl);
    opty.optimize(optimizer_strategy::MINIMIZE, 0, 1);

    std::cout << "P " << gl->get_optimizer_optimal_parameters()[0] << std::endl
        << "Q " << gl->get_optimizer_result() << std::endl;
    return 0;
}