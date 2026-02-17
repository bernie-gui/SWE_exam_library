#include <algorithm>
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
#include "io/array_parser.hpp"
#include "io/lambda_parser.hpp"
#include "montecarlo.hpp"
#include "network/message.hpp"
#include "process.hpp"
#include "simulator.hpp"
#include "system.hpp"
#include "utils/customer-server/server.hpp"
#include "utils/rate.hpp"
using namespace isw;

const int TIMESTEP = 1;

class requests_global : public global_t {
    public:
        size_t n, k;
        double a;
        utils::rate_meas_t q;
        std::vector<double> p, f;
        void init() override {
            global_t::init();
            q.init();
            // measure_v.init();
            // measure_w.init();
        }
};


class my_msg : public network::message_t {
    public:
        size_t item;
        bool success;
};


// class cust_thread_1 : public thread_t {
//     public:
//         void fun() override {
//             // size_t server;
//             // cs::request_t msg;
//             auto gl = get_global< requests_global >();
//             my_msg send;
//             send.item = gl->get_random()->uniform_range(0, gl->N);
//             send_message("Dispatchers", 0, send);
//             set_compute_time(gl->get_random()->uniform_range(gl->A, gl->B));
//         }
//         void init() override {
//             thread_t::init();
//             auto gl = get_global< requests_global >();
//             set_compute_time(gl->get_random()->uniform_range(gl->A, gl->B));
//         }
// };

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

int main()
{
    auto gl = std::make_shared< requests_global >();
    auto sys = std::make_shared< system_t >( gl );
    array_parser_t reader("examples/example_03_25_3.txt", {
        [gl](auto& iss) { double temp; iss >> temp; gl->set_horizon(temp); },
        [gl](auto& iss) { iss >> gl->n; },
        [gl](auto& iss) { iss >> gl->k;
                            gl->p.resize(gl->k);
                            gl->f.resize(gl->k); },
        [gl](auto &iss) { iss >> gl->a; },
        [gl](auto& iss) { for (size_t i = 0; i < gl->k; i++) iss >> gl->p[i]; },
        [gl](auto& iss) { for (size_t i = 0; i < gl->k; i++) iss >> gl->f[i]; },
    });
    reader.parse();
    // std::vector<size_t> database(gl->k);
    // std::fill(database.begin(), database.end(), 10);
    // for (size_t i = 0; i < gl->n; i++)
    //     sys->add_process( cs::server_t::create_process< my_msg >(
    //         gl->k * 2,
    //         [gl, &database](auto i){return i % 2 == 0 ? 0 : database[i/2];},
    //         TIMESTEP,
    //         {{"Customers", [gl](auto pt, auto msg){
    //             auto p = pt->template get_process<cs::server_t>();
    //             my_msg ret;
    //             double extract;
    //             if (p->database[msg->item/2] - p->database[msg->item/2+1] == 0) 
    //                 ret.success = false;
    //             else {
    //                 ret.success = true;
    //                 p->database[msg->item/2+1]++;
    //                 extract = gl->get_random()->uniform_range(0.0, 1.0);
    //                 if (extract <= gl->a) {
                        
    //                 }
    //             }
    //             ret.item = msg->item;
    //             assert(ret.server == pt->get_process()->get_relative_id().value());
    //             // ret.tag = msg->tag;
    //             // pt->send_message("Dispatchers", 0, ret);
    //         }}}
    //     ), "Servers" );
    // sys->add_process( std::make_shared< process_t >()->add_thread( std::make_shared< cust_thread_1 >() ), "Customers" );
    // sys->add_process(cs::server_t::create_process< my_msg >(
    //     0,
    //     [](auto){ return 0; },
    //     gl->D,
    //     {
    //         {"Customers", [gl](auto th, auto msg) { 
    //             my_msg msg2;
    //             if (msg->item == 0) return;
    //             msg2.item = msg->item-1;
    //             msg2.server = gl->get_random()->uniform_range(0, gl->S-1);
    //             th->send_message("Servers", msg2.server, msg2);
    //             gl->q.increase_denom(1);
    //         }},
    //         {"Servers", [gl](auto th, auto msg) {
    //             if (msg->success) return;
    //             auto &servers = th->get_process()->get_system()->template get_processes<cs::server_t>("Servers");
    //             std::shared_ptr<cs::server_t> one;
    //             gl->q.increase_amount(1);
    //             auto extract = gl->get_random()->uniform_range(0.0, 1.0);
    //             if (extract < gl->p) {
    //                 one = servers[msg->server];
    //                 one->database[msg->item]+=10;
    //             }
    //             else {
    //                 one = servers[gl->get_random()->uniform_range(0, gl->S-1)];
    //                 one->database[gl->get_random()->uniform_range(0, gl->N-1)]+=10;
    //             }
    //         }}}), "Dispatchers");
    // sys->add_network();

    // auto sim = std::make_shared< sim_help >( sys );
    // sim->run();
    

    // auto monty = montecarlo_t::create(sim);

    // monty->run();

    // std::cout << "Q " << gl->get_montecarlo_avg() << std::endl;
    // return 0;
}