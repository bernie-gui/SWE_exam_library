#include <algorithm>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <unordered_map>
#include <algorithm>
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
        size_t Q, P, S, C;
        double A, B, W, V, F;
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
            msg.item = gl->get_random()->uniform_range(0, gl->P-1);
            msg.quantity = gl->get_random()->uniform_range(1, gl->Q);
            msg.tag = _tag++;
            server = gl->get_random()->uniform_range(0, gl->S-1);
            p->request_history.emplace(std::make_tuple(server, msg.item, msg.tag), msg.quantity);
            send_message("Servers", server, msg);
            set_compute_time(gl->get_random()->uniform_range(gl->A, gl->B));
        }
        void init() override {
            thread_t::init();
            _tag = 0;
            auto gl = get_global< requests_global >();
            set_compute_time(gl->get_random()->uniform_range(gl->A, gl->B));
        }
    
    private:
        size_t _tag;
};

class cust_thread_2 : public thread_t {
    public:
        void fun() override {
            std::shared_ptr<cs::request_t> msg;
            auto p = get_process<cust>();
            auto gl = get_global<requests_global>();
            size_t quant;
            if ((msg = receive_message<cs::request_t>()) == nullptr) return;
            quant = msg->quantity;
            auto i = p->request_history.find({msg->sender_rel, msg->item, msg->tag});
            if (i != p->request_history.end()) {
                if (quant < i->second) {
                    gl->measure.update(1, get_thread_time());
                }
                p->request_history.erase(i);
            }
            else throw std::runtime_error(" unknown answer ");
        }
        cust_thread_2() : thread_t(1, 0, 1) {}
};

class sim_help : public simulator_t {
    public:
        void on_terminate() override {
            auto gl = get_global<requests_global>();
            gl->set_montecarlo_current(gl->measure.get_rate());
        }
        sim_help(std::shared_ptr<system_t> s) : simulator_t(s) {}
};

int main()
{
    auto gl = std::make_shared< requests_global >();
    auto sys = std::make_shared< system_t >( gl);
    auto reader = lambda_parser_t("examples/example_01_26_3.txt", {
        {"A", [gl](auto& iss) { iss >> gl->A; }},
        {"V", [gl](auto& iss) { iss >> gl->V; }},
        {"C", [gl](auto& iss) { iss >> gl->C; }},
        {"B", [gl](auto& iss) { iss >> gl->B; }},
        {"F", [gl](auto& iss) { iss >> gl->F; }},
        {"W", [gl](auto& iss) { iss >> gl->W; }},
        {"P", [gl](auto& iss) { iss >> gl->P; }},
        {"Q", [gl](auto& iss) { iss >> gl->Q; }},
        {"S", [gl](auto& iss) { iss >> gl->S; }},
        {"H", [gl](auto& iss) { double temp; iss >> temp; gl->set_horizon(temp); }},
        {"M", [gl](auto& iss) { double temp; iss >> temp; gl->set_montecarlo_budget(temp); }}}
    );
    size_t i;
    reader.parse();
    for (i = 0; i < gl->S; i++ ) {
        sys->add_process( cs::server_t::create_process< cs::request_t >(
            gl->P,
            [gl](auto){return gl->get_random()->uniform_range(0, gl->Q);},
            0.1,
            {{"Customers", [gl](auto pt, auto msg){
                auto p = pt->template get_process<cs::server_t>();
                auto copy = std::min(p->database[msg->item], msg->quantity);
                cs::request_t ret;
                // if (copy < msg->quantity) gl->measure.update(1, pt->get_thread_time());
                p->database[msg->item] -= copy;
                ret.quantity = copy;
                ret.item = msg->item;
                ret.tag = msg->tag;
                pt->send_message(msg->sender, ret);
            }},
            {"Suppliers", [](auto p, auto msg){
                auto pt = p->template get_process<cs::server_t>();
                pt->database[msg->item]+=msg->quantity;
            }}},
            0.2
        ), "Servers" );
    }
    for (i = 0; i < gl->C; i++) {
        sys->add_process( std::make_shared< cust >()->add_thread( std::make_shared< cust_thread_1 >() )->add_thread(std::make_shared<cust_thread_2>()), "Customers" );
    }
    sys->add_network();

    auto sim = std::make_shared< sim_help >( sys );

    auto monty = montecarlo_t::create(sim);

    std::cout << "Starting simulation..." << std::endl;

    monty->run();

    std::cout << "Simulation ended. Miss rate: " << gl->get_montecarlo_avg() << std::endl;

    return 0;
}