#include <cstdio>
#include "io/input_parser.hpp"
#include "io/output_writer.hpp"
#include "simulator.hpp"
#include "system.hpp"
#include "process.hpp"
#include "global.hpp"

using namespace isw;

class my_global : public global_t {
    public:
        double timestep, min_cust_sleep, max_cust_sleep;
        size_t products;
        output_writer_t writer{"examples/example-12-06-25_3_output.txt"};
};

class my_input : public input_parser_t {
    public:
        void parse() override {
            auto &stream = get_stream();
            auto global = get_global<my_global>();
            std::istringstream iss;
            std::string line;
            char tag;

            while(getline(stream, line)) {
                iss = std::istringstream(line);
                iss >> tag;
                switch (tag) {
                    case 'H': {
                        double hor;
                        iss >> hor;
                        global->set_horizon(hor);
                    } break;
                    case 'N': {
                        iss >> global->products;
                    } break;
                    case 'A': {
                        iss >> global->min_cust_sleep;
                    } break;
                    case 'B': {
                        iss >> global->max_cust_sleep;
                    } break;
                    case 'T': {
                        iss >> global->timestep;
                    } break;
                    default: exit(1);
                }
            }
        }

        my_input(std::shared_ptr<my_global> global) : input_parser_t(global, "examples/example-12-06-25_3_input.txt") {}
};

class cust_thread : public thread_t {
    public:

        void fun() override {
            auto gl = get_global<my_global>();
            if (get_thread_time() <= get_global()->get_horizon())
                gl->writer << get_thread_time() << " " << gl->get_random()->uniform_range(0, gl->products) << std::endl;
            set_sleep_time(gl->get_random()->uniform_range(gl->min_cust_sleep, gl->max_cust_sleep));
        }
};

int main() {
    auto global = std::make_shared<my_global>();
    auto input = std::make_shared<my_input>(global);
    input->parse();
    auto system = std::make_shared<system_t>(global);
    auto cust = std::make_shared<process_t>();
    cust->add_thread(std::make_shared<cust_thread>());
    system->add_process(cust, "customers");
    auto sim = std::make_shared<simulator_t>(system);
    sim->run();
}
