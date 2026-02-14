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
#include "io/output_writer.hpp"
using namespace isw;

class requests_global : public global_t {
    public:
        size_t K, P, S, C;
        double A, B, W, V, T, p;
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
            msg.quantity = 1;
            msg.tag = _tag++;
            server = 0;
            // p->request_history.emplace(std::make_tuple(server, msg.item, msg.tag), msg.quantity);
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
        size_t _tag; //should be an array
};

class cust_thread_2 : public thread_t {
    public:
        void fun() override {
            receive_message<cs::request_t>();
        }
        cust_thread_2() : thread_t(1, 0, 1) {}
        
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
    auto reader = lambda_parser_t("examples/example_11_25_4.txt", {
        {"A", [gl](auto& iss) { iss >> gl->A; }},
        {"V", [gl](auto& iss) { iss >> gl->V; }},
        {"B", [gl](auto& iss) { iss >> gl->B; }},
        {"W", [gl](auto& iss) { iss >> gl->W; }},
        {"P", [gl](auto& iss) { iss >> gl->P; }},
        {"p", [gl](auto& iss) { iss >> gl->p; }},
        {"K", [gl](auto& iss) { iss >> gl->K; }},
        {"T", [gl](auto& iss) { iss >> gl->T; }},
        {"H", [gl](auto& iss) { double temp; iss >> temp; gl->set_horizon(temp); }},
        {"M", [gl](auto& iss) { double temp; iss >> temp; gl->set_montecarlo_budget(temp); }}}
    );
    reader.parse();
    sys->add_process( cs::server_t::create_process< cs::request_t >(
        gl->P,
        [gl](auto){return gl->get_random()->uniform_range(0, gl->K);},
        gl->get_random()->uniform_range(gl->V, gl->W),
        {{"Customers", [gl](auto pt, auto msg){
            auto p = pt->template get_process<cs::server_t>();
            int copy = msg->quantity > (int) p->database[msg->item] ? -1 : 1;
            cs::request_t ret;
            if (copy == -1) gl->measure.update(1, pt->get_thread_time());
            else p->database[msg->item]--;
            ret.quantity = copy;
            ret.item = msg->item;
            ret.tag = msg->tag;
            pt->send_message(msg->sender, ret);
            double prob = gl->get_random()->uniform_range(0.0, 1.0);
            if (prob < gl->p) {
                p->database[msg->item] = gl->get_random()->uniform_range(0, gl->K);
            }
        }}},
        [gl](){return gl->get_random()->uniform_range(gl->V, gl->W);}
    ), "Servers" );
    sys->add_process( std::make_shared< cust >()->add_thread( std::make_shared< cust_thread_1 >() )->add_thread(std::make_shared<cust_thread_2>()), "Customers" );
    sys->add_network();

    auto sim = std::make_shared< sim_help >( sys );

    auto monty = montecarlo_t::create(sim);

    monty->run();

    output_writer_t out("examples/example_11_25_4out.txt");
    out << "2025-01-09-Mario-Rossi-1234567" << std::endl 
        << "R " << gl->get_montecarlo_avg() << std::endl;

    return 0;
}