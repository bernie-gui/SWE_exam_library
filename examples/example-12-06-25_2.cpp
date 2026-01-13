#include <cstddef>
#include <memory>
#include <sstream>
#include <cmath>
#include <iostream>
#include "global.hpp"
#include "io/input_parser.hpp"
#include "io/output_writer.hpp"
#include "montecarlo.hpp"
#include "process.hpp"
#include "simulator.hpp"
#include "system.hpp"

using namespace isw;

class vehicle_global : public global_t {
    public:
        size_t vehicle_number, collision_idx = 0;
        double speed, sleep, radius,
            last_collision_time, collision_time_avg;
        output_writer_t writer = output_writer_t("examples/example-12-06-25_2_output.txt");

        void init() override {
            last_collision_time = 0;
            collision_time_avg = 0;
        }
};

//output writer buffer

class vehicle_input : public input_parser_t {
    public:
        void parse() override {
            auto global = get_global<vehicle_global>();
            auto &stream = get_stream();
            std::string line;
            std::istringstream iss;
            char tag;

            std::getline(stream, line);
            iss = std::istringstream(line);
            double horizon;
            iss >> tag >> horizon;
            global->set_horizon(horizon);

            std::getline(stream, line);
            iss = std::istringstream(line);
            iss >> tag >> global->vehicle_number;

            std::getline(stream, line);
            iss = std::istringstream(line);
            iss >> tag >> global->speed;

            std::getline(stream, line);
            iss = std::istringstream(line);
            iss >> tag >> global->sleep;

            std::getline(stream, line);
            iss = std::istringstream(line);
            iss >> tag >> global->radius;
        }

        vehicle_input(std::shared_ptr<vehicle_global> global) : input_parser_t(global, "examples/example-12-06-25_2_input.txt") {}
};

class vehicle : public process_t {
    public:
        double x, y;

        void init() override {
            process_t::init();
            auto gl = get_global();
            x = gl->get_random()->uniform_range(-10.0, 10.0);
            y = gl->get_random()->uniform_range(-10.0, 10.0);
        }
};

class vehicle_thread : public thread_t {
    public:

        void init() override {
            thread_t::init();
            auto gl = get_global<vehicle_global>();
            set_sleep_time(gl->sleep);
        }

        void fun() override {
            auto gl = get_global<vehicle_global>();
            double th = gl->get_random()->uniform_range(.0, 2 * M_PI);
            auto v = get_process<vehicle>();
            v->x += gl->speed * gl->sleep * sin(th);
            v->y += gl->speed * gl->sleep * cos(th);
        }
};

class vehicle_1_thread : public thread_t {
public:

    void init() override {
        thread_t::init();
        auto gl = get_global<vehicle_global>();
        set_sleep_time(gl->sleep);
    }

    void fun() override {
        auto gl = get_global<vehicle_global>();
        auto my_v = get_process<vehicle>();
        auto &vs = my_v->get_system()->get_processes<vehicle>();
        size_t q[4], candidate;
        size_t min = std::numeric_limits<size_t>::infinity();
        for (auto &v : vs) {
            if (v->get_id().value() == my_v->get_id().value()) continue;
            if (std::sqrt(std::pow(v->x - my_v->x, 2) + std::pow(v->y - my_v->y, 2)) <= gl->radius) {
                if (v->x - my_v->x >= 0 && v->y - my_v->y >= 0) q[0]++;
                else if (v->x - my_v->x >= 0 && v->y - my_v->y <= 0) q[1]++;
                else if (v->x - my_v->x <= 0 && v->y - my_v->y <= 0) q[2]++;
                else if (v->x - my_v->x <= 0 && v->y - my_v->y >= 0) q[3]++;
            }
        }
        for (size_t i = 0; i < 4; i++) {
            if (q[i] < min) {
                min = q[i];
                candidate = i;
            }
        }
        double th = gl->get_random()->uniform_range(candidate * (M_PI /2), (candidate + 1) * (M_PI / 2));
        my_v->x += gl->speed * gl->sleep * sin(th);
        my_v->y += gl->speed * gl->sleep * cos(th);
    }
};


class my_sys : public system_t {
    public:
        void on_end_step() override {
            auto gl = get_global<vehicle_global>();
            auto &vs = get_processes<vehicle>();
            auto &first = vs[0];
            for (auto &v : get_processes<vehicle>()) {
                if (v->get_id().value() == 0) continue;
                if (std::sqrt(std::pow(first->x - v->x, 2) + std::pow(first->y - v->y, 2)) <= gl->radius) {
                    auto gl = get_global<vehicle_global>();
                    gl->collision_idx++;
                    double coll_diff = get_current_time() - gl->last_collision_time;
                    gl->collision_time_avg = (((gl-> collision_time_avg * (gl->collision_idx - 1)) + coll_diff)
                    / gl->collision_idx);
                    gl->last_collision_time = get_current_time();
                }
            }
        }

        my_sys(std::shared_ptr<vehicle_global> global) : system_t(global, "road-rage") {}
};

class my_sim : public simulator_t {
    public:
        void on_terminate() override {
            auto gl = get_global<vehicle_global>();
            gl->set_montecarlo_current(gl->collision_time_avg);
        }

        // bool should_terminate() override {
        //     bool x = simulator_t::should_terminate();
        //     std::cout << ((get_system()->get_current_time() >= get_global()->get_horizon()) ? "halt" : "ok") << std::endl;
        //     std::cout << (x ? "stop" : "go") <<std::endl;
        //     return x;
        // }

        my_sim(const std::shared_ptr< system_t > sys) : simulator_t(sys) {}
};

// global should reset montecarlo current?

int main() {

    auto gl = std::make_shared<vehicle_global>();
    gl->writer.write_line("Ao");
    gl->set_montecarlo_budget(1000);
    vehicle_input reader(gl);
    reader.parse();
    auto sys = std::make_shared<my_sys>(gl);
    std::shared_ptr<thread_t> thread;
    std::shared_ptr<vehicle> process;
    for (size_t i = 0; i < gl->vehicle_number; i++) {
        if (i == 0) thread = std::make_shared<vehicle_1_thread>();
        else thread = std::make_shared<vehicle_thread>();
        process = std::make_shared<vehicle>();
        sys->add_process(process->add_thread(thread));
    }
    auto sim = std::make_shared<my_sim>(sys);
    auto monty = montecarlo_t::create(sim);
    monty->run();
    std::cout << gl->collision_time_avg << std::endl;
}
