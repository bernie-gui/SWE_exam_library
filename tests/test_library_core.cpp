#include "catch_lib.hpp"

/**
 * @file test_library_core.cpp
 * @brief Unit tests for the ISW library core components
 * @details Exercise-independent tests covering the fundamental library classes:
 *          random_t, global_t, system_t, process_t, thread_t, simulator_t,
 *          input_parser_t, lambda_parser_t, output_writer_t, logger_t,
 *          markov_chain, rate_meas_t.
 */

#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "common.hpp"
#include "global.hpp"
#include "montecarlo.hpp"
#include "process.hpp"
#include "random.hpp"
#include "simulator.hpp"
#include "system.hpp"
#include "io/input_parser.hpp"
#include "io/lambda_parser.hpp"
#include "io/logger.hpp"
#include "io/output_writer.hpp"
#include "utils/markov/markov.hpp"
#include "utils/rate.hpp"

using namespace isw;

// ============================================================================
// Minimal helpers (exercise-independent)
// ============================================================================

class noop_thread_t : public thread_t {
public:
    noop_thread_t() : thread_t(0, 0, 0) {}
    void fun() override {}
};

// ============================================================================
// SECTION 1: random_t
// ============================================================================

TEST_CASE("random_t: uniform_range(double) stays in bounds", "[random]") {
    random_t rng(42);
    for (int i = 0; i < 10000; ++i) {
        double v = rng.uniform_range(0.0, 1.0);
        REQUIRE(v >= 0.0);
        REQUIRE(v < 1.0);
    }
}

TEST_CASE("random_t: uniform_range(int) stays in bounds", "[random]") {
    random_t rng(42);
    for (int i = 0; i < 10000; ++i) {
        int v = rng.uniform_range(0, 10);
        REQUIRE(v >= 0);
        REQUIRE(v <= 10);
    }
}

TEST_CASE("random_t: seeded engine is reproducible", "[random]") {
    random_t a(123), b(123);
    for (int i = 0; i < 100; ++i)
        REQUIRE(a.uniform_range(0.0, 1.0) == b.uniform_range(0.0, 1.0));
}

TEST_CASE("random_t: gaussian_sample produces finite values", "[random]") {
    random_t rng(99);
    double sum = 0;
    int N = 10000;
    for (int i = 0; i < N; ++i) {
        double v = rng.gaussian_sample(100.0, 15.0);
        REQUIRE(std::isfinite(v));
        sum += v;
    }
    double mean = sum / N;
    // Mean should be close to 100.0 (law of large numbers)
    REQUIRE(std::abs(mean - 100.0) < 3.0);
}

TEST_CASE("random_t: different seeds produce different sequences", "[random]") {
    random_t a(1), b(2);
    bool all_same = true;
    for (int i = 0; i < 50; ++i) {
        if (a.uniform_range(0.0, 1.0) != b.uniform_range(0.0, 1.0)) {
            all_same = false;
            break;
        }
    }
    REQUIRE_FALSE(all_same);
}

// ============================================================================
// SECTION 2: global_t
// ============================================================================

TEST_CASE("global_t: default construction and getters/setters", "[global]") {
    auto g = std::make_shared<global_t>();

    SECTION("random is not null") {
        REQUIRE(g->get_random() != nullptr);
    }

    SECTION("montecarlo budget round-trips") {
        g->set_montecarlo_budget(500);
        REQUIRE(g->montecarlo_budget() == 500);
    }

    SECTION("horizon round-trips") {
        g->set_horizon(42.5);
        REQUIRE(g->get_horizon() == Catch::Approx(42.5));
    }

    SECTION("montecarlo avg round-trips") {
        g->set_montecarlo_avg(3.14);
        REQUIRE(g->get_montecarlo_avg() == Catch::Approx(3.14));
    }

    SECTION("montecarlo current round-trips") {
        g->set_montecarlo_current(7.77);
        REQUIRE(g->montecarlo_current() == Catch::Approx(7.77));
    }

    SECTION("optimizer result round-trips") {
        g->set_optimizer_result(99.0);
        REQUIRE(g->get_optimizer_result() == Catch::Approx(99.0));
    }

    SECTION("optimizer optimal parameters round-trip") {
        std::vector<double> params = {1.0, 2.0, 3.0};
        g->set_optimizer_optimal_parameters(params);
        auto got = g->get_optimizer_optimal_parameters();
        REQUIRE(got.size() == 3);
        REQUIRE(got[0] == Catch::Approx(1.0));
        REQUIRE(got[2] == Catch::Approx(3.0));
    }

    SECTION("init resets montecarlo_current to 0") {
        g->set_montecarlo_current(123.0);
        g->init();
        REQUIRE(g->montecarlo_current() == Catch::Approx(0.0));
    }
}

TEST_CASE("global_t: channels resize with processes", "[global]") {
    auto g = std::make_shared<global_t>();
    auto sys = system_t::create(g, "chan_test");
    auto p1 = process_t::create("p1");
    auto p2 = process_t::create("p2");

    p1->add_thread(std::make_shared<noop_thread_t>());
    p2->add_thread(std::make_shared<noop_thread_t>());

    sys->add_process(p1);
    sys->add_process(p2);

    REQUIRE(g->get_channel_in().size() == 2);
    REQUIRE(g->get_channel_out().size() == 2);
}

// ============================================================================
// SECTION 3: system_t / process_t / thread_t basics
// ============================================================================

TEST_CASE("system_t: create and add processes", "[system]") {
    auto g = std::make_shared<global_t>();
    auto sys = system_t::create(g, "test_sys");

    auto p = process_t::create("proc_a");
    p->add_thread(std::make_shared<noop_thread_t>());
    sys->add_process(p);

    REQUIRE(sys->get_processes().size() == 1);
    REQUIRE(p->get_id().has_value());
    REQUIRE(p->get_id().value() == 0);
}

TEST_CASE("system_t: world management", "[system]") {
    auto g = std::make_shared<global_t>();
    auto sys = system_t::create(g, "world_test");

    auto pa = process_t::create("a");
    pa->add_thread(std::make_shared<noop_thread_t>());
    auto pb = process_t::create("b");
    pb->add_thread(std::make_shared<noop_thread_t>());
    auto pc = process_t::create("c");
    pc->add_thread(std::make_shared<noop_thread_t>());

    sys->add_process(pa, "world_A");
    sys->add_process(pb, "world_A");
    sys->add_process(pc, "world_B");

    REQUIRE(sys->total_worlds() == 2);
    REQUIRE(sys->world_size("world_A") == 2);
    REQUIRE(sys->world_size("world_B") == 1);

    SECTION("get_abs_id / get_rel_id round-trip") {
        size_t abs = sys->get_abs_id("world_A", 1);
        auto rel = sys->get_rel_id(abs);
        REQUIRE(rel.world == "world_A");
        REQUIRE(rel.rel_id == 1);
    }

    SECTION("get_abs_id throws for invalid world") {
        REQUIRE_THROWS_AS(sys->get_abs_id("nonexistent", 0), std::out_of_range);
    }

    SECTION("get_abs_id throws for out-of-range rel_id") {
        REQUIRE_THROWS_AS(sys->get_abs_id("world_A", 99), std::out_of_range);
    }
}

TEST_CASE("thread_t: timing properties", "[thread]") {
    class timed_thread : public thread_t {
    public:
        int call_count = 0;
        timed_thread() : thread_t(1.0, 2.0, 0.0) {}
        void fun() override {
            call_count++;
            set_thread_time(get_thread_time() + get_compute_time() + get_sleep_time());
        }
    };

    auto g = std::make_shared<global_t>();
    g->set_horizon(10.0);
    auto sys = system_t::create(g, "timing_test");
    auto proc = process_t::create("timed_proc");
    auto th = std::make_shared<timed_thread>();
    proc->add_thread(th);
    sys->add_process(proc);

    REQUIRE(th->get_compute_time() == Catch::Approx(1.0));
    REQUIRE(th->get_sleep_time() == Catch::Approx(2.0));
    REQUIRE(th->get_thread_time() == Catch::Approx(0.0));
}

TEST_CASE("process_t: active flag", "[process]") {
    auto p = process_t::create("flag_test");
    p->add_thread(std::make_shared<noop_thread_t>());

    auto g = std::make_shared<global_t>();
    auto sys = system_t::create(g, "active_test");
    sys->add_process(p);

    // After init, process should be active
    sys->init();
    REQUIRE(p->is_active());

    p->set_active(false);
    REQUIRE_FALSE(p->is_active());
}

TEST_CASE("process_t: deactivated process is skipped by system step", "[process]") {
    class counting_thread_t : public thread_t {
    public:
        int call_count = 0;
        counting_thread_t() : thread_t(0, 0, 0) {}
        void fun() override {
            call_count++;
            set_thread_time(get_thread_time() + 1.0);
        }
    };

    auto g = std::make_shared<global_t>();
    g->set_horizon(5.0);

    SECTION("Active process runs its threads") {
        auto sys = system_t::create(g, "active_test");
        auto proc = process_t::create("test_proc");
        auto th = std::make_shared<counting_thread_t>();
        proc->add_thread(th);
        sys->add_process(proc);

        auto sim = std::make_shared<simulator_t>(sys);
        sim->run();
        REQUIRE(th->call_count >= 4);
    }

    SECTION("Deactivated process stops being scheduled") {
        auto sys = system_t::create(g, "deactivate_test");
        auto proc = process_t::create("test_proc");
        auto th = std::make_shared<counting_thread_t>();
        proc->add_thread(th);
        sys->add_process(proc);

        // Init, let it run once, then deactivate
        sys->init();
        sys->step();
        int after_one_step = th->call_count;
        REQUIRE(after_one_step >= 1);

        proc->set_active(false);
        // Additional steps should not schedule this process
        sys->step();
        sys->step();
        sys->step();
        REQUIRE(th->call_count == after_one_step);
    }
}

// ============================================================================
// SECTION 4: simulator_t
// ============================================================================

TEST_CASE("simulator_t: default terminates at horizon", "[simulator]") {
    auto g = std::make_shared<global_t>();
    g->set_horizon(1.0);

    class step_thread : public thread_t {
    public:
        int steps = 0;
        step_thread() : thread_t(0, 0, 0) {}
        void fun() override {
            steps++;
            set_thread_time(get_thread_time() + 0.3);
        }
    };

    auto sys = system_t::create(g, "horizon_test");
    auto proc = process_t::create("stepper");
    auto th = std::make_shared<step_thread>();
    proc->add_thread(th);
    sys->add_process(proc);

    auto sim = std::make_shared<simulator_t>(sys);
    sim->run();

    // With dt=0.3 and horizon=1.0, should step ~4 times (0, 0.3, 0.6, 0.9)
    REQUIRE(th->steps >= 3);
    REQUIRE(th->steps <= 5);
}

TEST_CASE("simulator_t: on_terminate callback fires", "[simulator]") {
    // A simple global that the thread sets a flag on
    class flag_global_t : public global_t {
    public:
        bool done = false;
        void init() override {
            global_t::init();
            done = false;
        }
    };

    class flag_thread_t : public thread_t {
    public:
        flag_thread_t() : thread_t(0, 0, 0) {}
        void fun() override {
            get_global<flag_global_t>()->done = true;
        }
    };

    class counting_simulator_t : public simulator_t {
    public:
        int terminate_count = 0;
        using simulator_t::simulator_t;
        bool should_terminate() override {
            return get_global<flag_global_t>()->done;
        }
        void on_terminate() override {
            terminate_count++;
        }
    };

    auto global = std::make_shared<flag_global_t>();
    auto sys = system_t::create(global, "terminate_test");
    auto proc = process_t::create("worker");
    proc->add_thread(std::make_shared<flag_thread_t>());
    sys->add_process(proc);

    auto sim = std::make_shared<counting_simulator_t>(sys);
    sim->run();

    REQUIRE(sim->terminate_count == 1);
}

TEST_CASE("simulator_t: on_terminate called once per montecarlo iteration", "[simulator][montecarlo]") {
    static int global_terminate_count = 0;
    global_terminate_count = 0;

    class flag_global_t : public global_t {
    public:
        bool done = false;
        void init() override {
            global_t::init();
            done = false;
            set_montecarlo_current(0.0);
        }
    };

    class flag_thread_t : public thread_t {
    public:
        flag_thread_t() : thread_t(0, 0, 0) {}
        void fun() override {
            auto g = get_global<flag_global_t>();
            g->done = true;
            g->set_montecarlo_current(1.0);
        }
    };

    class counting_sim_t : public simulator_t {
    public:
        using simulator_t::simulator_t;
        bool should_terminate() override {
            return get_global<flag_global_t>()->done;
        }
        void on_terminate() override {
            global_terminate_count++;
        }
    };

    auto global = std::make_shared<flag_global_t>();
    global->set_montecarlo_budget(50);

    auto sys = system_t::create(global, "mc_terminate_test");
    auto proc = process_t::create("worker");
    proc->add_thread(std::make_shared<flag_thread_t>());
    sys->add_process(proc);

    auto sim = std::make_shared<counting_sim_t>(sys);
    auto mc = montecarlo_t::create(sim);
    mc->run();

    REQUIRE(global_terminate_count == 50);
}

// ============================================================================
// SECTION 5: lambda_parser_t (generic features)
// ============================================================================

TEST_CASE("lambda_parser_t: unknown keys are silently ignored", "[lambda_parser][io]") {
    const std::string tmp = "tests/_tmp_lambda_test.txt";
    {
        std::ofstream f(tmp);
        f << "UNKNOWN_KEY 42\n";
        f << "VAL 7\n";
    }

    int parsed_val = 0;
    lambda_parser_t lp(tmp, std::unordered_map<std::string, parser>{
        {"VAL", [&](std::istringstream& iss) { iss >> parsed_val; }}
    });
    REQUIRE_NOTHROW(lp.parse());
    REQUIRE(parsed_val == 7);

    std::filesystem::remove(tmp);
}

TEST_CASE("lambda_parser_t: set_bindings replaces bindings", "[lambda_parser][io]") {
    const std::string tmp = "tests/_tmp_setbind_test.txt";
    {
        std::ofstream f(tmp);
        f << "X 10\n";
        f << "Y 20\n";
    }

    int x = 0, y = 0;

    // Start with empty bindings
    lambda_parser_t lp(tmp, std::unordered_map<std::string, parser>{});
    lp.parse();
    REQUIRE(x == 0); // nothing parsed

    // Set proper bindings and re-parse
    lp.set_bindings(std::unordered_map<std::string, parser>{
        {"X", [&](std::istringstream& iss) { iss >> x; }},
        {"Y", [&](std::istringstream& iss) { iss >> y; }}
    });
    lp.reset_stream();
    lp.parse();
    REQUIRE(x == 10);
    REQUIRE(y == 20);

    std::filesystem::remove(tmp);
}

// ============================================================================
// SECTION 6: output_writer_t
// ============================================================================

TEST_CASE("output_writer_t: write_line and stream operator", "[output_writer][io]") {
    const std::string tmp = "tests/_tmp_output.txt";
    {
        output_writer_t writer(tmp);
        writer.write_line("2025-01-09-Test-Line1");
        writer << "C " << 285.5 << std::endl;
    }

    std::ifstream in(tmp);
    REQUIRE(in.is_open());
    std::string line1, line2;
    std::getline(in, line1);
    std::getline(in, line2);

    REQUIRE(line1 == "2025-01-09-Test-Line1");
    REQUIRE(line2.substr(0, 2) == "C ");
    double val = std::stod(line2.substr(2));
    REQUIRE(val == Catch::Approx(285.5));

    in.close();
    std::filesystem::remove(tmp);
}

TEST_CASE("output_writer_t: write result format", "[output_writer][io]") {
    const std::string tmp = "tests/_tmp_result_format.txt";
    {
        output_writer_t writer(tmp);
        writer.write_line("2025-01-09-Mario-Rossi-1234567");
        writer << "V " << 42.0 << std::endl;
    }

    std::ifstream in(tmp);
    std::string line1, line2;
    std::getline(in, line1);
    std::getline(in, line2);

    REQUIRE(line1.find("2025-01-09") != std::string::npos);
    REQUIRE(line2[0] == 'V');
    REQUIRE(line2[1] == ' ');

    in.close();
    std::filesystem::remove(tmp);
}

TEST_CASE("output_writer_t: writes arbitrary key-value lines", "[output_writer][io]") {
    const std::string tmp = "tests/_tmp_output_kv.txt";
    {
        output_writer_t writer(tmp);
        writer.write_line("header-line");
        writer << "P " << 0.297 << std::endl;
    }

    std::ifstream in(tmp);
    REQUIRE(in.is_open());

    std::string line1, line2;
    std::getline(in, line1);
    std::getline(in, line2);

    REQUIRE(line1 == "header-line");
    REQUIRE(line2[0] == 'P');
    REQUIRE(line2[1] == ' ');
    double val = std::stod(line2.substr(2));
    REQUIRE(val == Catch::Approx(0.297).margin(0.001));

    in.close();
    std::filesystem::remove(tmp);
}

// ============================================================================
// SECTION 7: logger_t
// ============================================================================

TEST_CASE("logger_t: create and add fields", "[logger][io]") {
    const std::string tmp = "tests/_tmp_log.csv";
    {
        auto logger = logger_t::create(tmp);
        logger->add_field("time")->add_field("cost")->add_field("state");
        REQUIRE_NOTHROW(logger->log_fields());
    }
    std::filesystem::remove(tmp);
}

TEST_CASE("logger_t: throws on double log_fields", "[logger][io]") {
    const std::string tmp = "tests/_tmp_log2.csv";
    {
        auto logger = logger_t::create(tmp);
        logger->add_field("x")->log_fields();
        REQUIRE_THROWS(logger->log_fields());
    }
    std::filesystem::remove(tmp);
}

TEST_CASE("logger_t: throws on add_field after log_fields", "[logger][io]") {
    const std::string tmp = "tests/_tmp_log3.csv";
    {
        auto logger = logger_t::create(tmp);
        logger->add_field("x")->log_fields();
        REQUIRE_THROWS(logger->add_field("y"));
    }
    std::filesystem::remove(tmp);
}

TEST_CASE("logger_t: log_measurement with mismatched count throws", "[logger][io]") {
    const std::string tmp = "tests/_tmp_log4.csv";
    {
        auto logger = logger_t::create(tmp);
        logger->add_field("a")->add_field("b")->log_fields();
        logger->add_measurement("1");
        REQUIRE_THROWS(logger->log_measurement());
    }
    std::filesystem::remove(tmp);
}

TEST_CASE("logger_t: full logging workflow", "[logger][io]") {
    const std::string tmp = "tests/_tmp_log5.csv";
    {
        auto logger = logger_t::create(tmp);
        logger->add_field("iteration")
              ->add_field("cost")
              ->log_fields();

        for (int i = 0; i < 3; ++i) {
            logger->add_measurement(std::to_string(i))
                  ->add_measurement(std::to_string(i * 100.0))
                  ->log_measurement();
        }
    }
    REQUIRE(std::filesystem::exists(tmp));
    REQUIRE(std::filesystem::file_size(tmp) > 0);
    std::filesystem::remove(tmp);
}

// ============================================================================
// SECTION 8: markov_chain utility
// ============================================================================

TEST_CASE("markov_chain: deterministic transitions", "[markov]") {
    isw::markov::markov_chain_t mc(3);
    mc.matrix[0][1] = {1.0, 50.0};
    mc.matrix[1][2] = {1.0, 30.0};
    mc.matrix[2][2] = {1.0, 0.0};

    random_t rng(42);
    REQUIRE(mc.next_state(0, rng.get_engine()) == 1);
    REQUIRE(mc.next_state(1, rng.get_engine()) == 2);
    REQUIRE(mc.next_state(2, rng.get_engine()) == 2);
}

TEST_CASE("markov_chain: probabilistic transitions converge", "[markov]") {
    isw::markov::markov_chain_t mc(2);
    mc.matrix[0][0] = {0.3, 0.0};
    mc.matrix[0][1] = {0.7, 0.0};
    mc.matrix[1][1] = {1.0, 0.0};

    random_t rng(42);
    int count_to_1 = 0;
    int N = 10000;
    for (int i = 0; i < N; ++i) {
        if (mc.next_state(0, rng.get_engine()) == 1)
            count_to_1++;
    }
    double observed = static_cast<double>(count_to_1) / N;
    REQUIRE(observed == Catch::Approx(0.7).margin(0.03));
}

TEST_CASE("markov_chain: cost stored in matrix", "[markov]") {
    isw::markov::markov_chain_t mc(2);
    mc.matrix[0][1] = {1.0, 42.0};
    mc.matrix[1][1] = {1.0, 0.0};

    REQUIRE(mc.matrix[0][1].second == Catch::Approx(42.0));
}

// ============================================================================
// SECTION 9: rate_meas_t utility
// ============================================================================

TEST_CASE("rate_meas_t: basic rate measurement", "[rate]") {
    isw::utils::rate_meas_t rate;
    REQUIRE(rate.get_rate() == Catch::Approx(0.0));

    rate.update(10.0, 1.0);
    REQUIRE(rate.get_rate() == Catch::Approx(10.0));

    rate.update(20.0, 2.0);
    // rate = old_rate * (old_time / new_time) + amount / new_time
    // = 10.0 * (1.0 / 2.0) + 20.0 / 2.0 = 5.0 + 10.0 = 15.0
    REQUIRE(rate.get_rate() == Catch::Approx(15.0));
}

TEST_CASE("rate_meas_t: init resets rate", "[rate]") {
    isw::utils::rate_meas_t rate;
    rate.update(10.0, 1.0);
    REQUIRE(rate.get_rate() != Catch::Approx(0.0));
    rate.init();
    REQUIRE(rate.get_rate() == Catch::Approx(0.0));
}

TEST_CASE("rate_meas_t: update at time zero throws", "[rate]") {
    isw::utils::rate_meas_t rate;
    REQUIRE_THROWS(rate.update(10.0, 0.0));
}
