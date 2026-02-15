#include <cstddef>
#include <cstring>
#include <iostream>
#include <memory>
#include <tuple>
#include <unordered_map>
#include "utils/customer-server/utils.hpp"
#include "utils/customer-server/server.hpp"
#include "global.hpp"
#include "io/lambda_parser.hpp"
#include "montecarlo.hpp"
#include "process.hpp"
#include "simulator.hpp"
#include "system.hpp"
#include "utils/rate.hpp"
using namespace isw;

class requests_global : public global_t {
    public:
        size_t K, N, S, C, Q;
        double A, B, W, V, T, p, F, G;
        utils::rate_meas_t measure;
        void init() override {
            global_t::init();
            measure.init();
        }
};

class cust : public process_t {
    public:
        std::unordered_map<std::tuple<size_t, size_t, size_t>, size_t> request_history;
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
            msg.item = gl->get_random()->uniform_range(0, gl->N-1);
            msg.quantity = gl->get_random()->uniform_range(1, gl->K);
            server = gl->get_random()->uniform_range(0, gl->S-1);;
            // p->request_history.emplace(std::make_tuple(server, msg.item, msg.tag), msg.quantity);
            send_message("Servers", server, msg);
            auto msg2 = receive_message<cs::request_t>();
            if (msg2 && msg2->quantity == -1) {
                gl->measure.update(1, get_thread_time());
            }
            set_compute_time(gl->get_random()->uniform_range(gl->A, gl->B));
        }
        void init() override {
            thread_t::init();
            auto gl = get_global< requests_global >();
            set_compute_time(gl->get_random()->uniform_range(gl->A, gl->B));
        }
};

class sim_help : public simulator_t {
    public:
        void on_terminate() override {
            auto gl = get_global<requests_global>();
            // gl->measure.update(0, gl->get_horizon()); //tecnicamente corretto ma cambia poco
            gl->set_montecarlo_current(gl->measure.get_rate()); 
        }
        sim_help(std::shared_ptr<system_t> s) : simulator_t(s) {}
};

int main()
{
    auto gl = std::make_shared< requests_global >();
    auto sys = std::make_shared< system_t >( gl);
    auto reader = lambda_parser_t("examples/example_07_25_3.txt", {
        {"A", [gl](auto& iss) { iss >> gl->A; }},
        {"B", [gl](auto& iss) { iss >> gl->B; }},
        {"N", [gl](auto& iss) { iss >> gl->N; }},
        {"p", [gl](auto& iss) { iss >> gl->p; }},
        {"K", [gl](auto& iss) { iss >> gl->K; }},
        {"F", [gl](auto& iss) { iss >> gl->F; }},
        {"Q", [gl](auto& iss) { iss >> gl->Q; }},
        {"G", [gl](auto& iss) { iss >> gl->G; }},
        {"T", [gl](auto& iss) { iss >> gl->T; }},
        {"S", [gl](auto& iss) { iss >> gl->S; }},
        {"H", [gl](auto& iss) { double temp; iss >> temp; gl->set_horizon(temp); }},
        {"M", [gl](auto& iss) { double temp; iss >> temp; gl->set_montecarlo_budget(temp); }}}
    );
    reader.parse();
    for (size_t i = 0; i < gl->S; i++)
        sys->add_process( cs::server_t::create_process< cs::request_t >(
            gl->N,
            [gl](auto){return gl->get_random()->uniform_range(0, gl->K);},
            gl->F,
            {{"Customers", [gl](auto pt, auto msg){
                auto p = pt->template get_process<cs::server_t>();
                int copy;
                if (msg->quantity > (int) p->database[msg->item]) copy = -1;
                else {
                    copy = msg->quantity;
                    p->database[msg->item] -= copy;
                }
                cs::request_t ret;
                // if (copy == -1) gl->measure.update(1, pt->get_thread_time());
                ret.quantity = copy;
                ret.item = msg->item;
                // ret.tag = msg->tag;
                pt->send_message(msg->sender, ret);
                double prob = gl->get_random()->uniform_range(0.0, 1.0);
                if (prob < gl->p) {
                    p->database[msg->item] += gl->Q;
                    pt->set_thread_time(gl->G-gl->F);
                }
            }}}
        ), "Servers" );
    sys->add_process( std::make_shared< cust >()->add_thread( std::make_shared< cust_thread_1 >() ), "Customers" );
    sys->add_network();

    auto sim = std::make_shared< sim_help >( sys );

    auto monty = montecarlo_t::create(sim);

    monty->run();

    std::cout << "R " << gl->get_montecarlo_avg() << std::endl;

    return 0;
}