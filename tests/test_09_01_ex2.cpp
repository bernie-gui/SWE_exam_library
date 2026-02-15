#include "catch_lib.hpp"

/**
 * @file test_09_01_ex2.cpp
 * @brief Unit tests for Exercise 09-01 Ex.2 — Probability Estimation via Monte Carlo
 * @details Exercise-specific tests for MDP probability estimation P(cost ≤ C):
 *          - Probability-mode montecarlo (indicator variable averaging)
 *          - input_parser_t and lambda_parser_t with "C" key for cost threshold
 *          - Custom inline and file-based probability estimation scenarios
 *          - Running average correctness for indicator variables
 *          - montecarlo_t::create<SIM> template factory
 *          - System reset between consecutive runs
 *          - Graded scenarios (tests 1 & 5)
 *          - Edge cases and convergence tests
 *          Generic library tests are in test_library_core.cpp.
 */

#include <cassert>
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
#include "io/output_writer.hpp"

using namespace isw;

// ============================================================================
// MDP helpers for Exercise 2 (probability estimation)
// ============================================================================

struct transition_t {
    size_t target;
    double prob;
    double cost;
};

class mdp_prob_global_t : public global_t {
public:
    mdp_prob_global_t() : global_t() {}

    size_t num_states = 0;
    double max_allowed_cost = 0.0;  // C threshold
    std::vector<std::vector<transition_t>> adj_list;

    double accumulated_cost = 0.0;
    bool reached_terminal = false;

    void init() override {
        global_t::init();
        accumulated_cost = 0.0;
        reached_terminal = false;
        set_montecarlo_current(0.0);
    }
};

class mdp_prob_thread_t : public thread_t {
private:
    size_t current_node;
public:
    mdp_prob_thread_t() : thread_t(0.0, 0.0, 0.0), current_node(0) {}

    void init() override {
        thread_t::init();
        current_node = 0;
    }

    void fun() override {
        auto global = get_global<mdp_prob_global_t>();

        if (current_node == global->num_states - 1) {
            global->reached_terminal = true;
            // Indicator: 1.0 if cost <= C, 0.0 otherwise
            double result = (global->accumulated_cost <= global->max_allowed_cost) ? 1.0 : 0.0;
            global->set_montecarlo_current(result);
            return;
        }

        const auto& transitions = global->adj_list[current_node];
        if (transitions.empty()) return;

        double r = global->get_random()->uniform_range(0.0, 1.0);
        double cumulative_prob = 0.0;

        for (const auto& trans : transitions) {
            cumulative_prob += trans.prob;
            if (r <= cumulative_prob) {
                global->accumulated_cost += trans.cost;
                current_node = trans.target;
                break;
            }
        }
        set_thread_time(get_thread_time() + 0.1);
    }
};

class mdp_prob_simulator_t : public simulator_t {
public:
    using simulator_t::simulator_t;
    bool should_terminate() override {
        return get_global<mdp_prob_global_t>()->reached_terminal;
    }
};

// Custom parser using inheritance
class mdp_prob_input_parser_t : public input_parser_t {
private:
    std::shared_ptr<mdp_prob_global_t> _global;
public:
    mdp_prob_input_parser_t(const std::string& filename, std::shared_ptr<mdp_prob_global_t> global)
        : input_parser_t(filename), _global(global) {}

    void parse() override {
        auto& iss = get_stream();
        std::string token;
        while (iss >> token) {
            if (token == "C") {
                double c; iss >> c;
                _global->max_allowed_cost = c;
            } else if (token == "N") {
                size_t n; iss >> n;
                _global->num_states = n;
                _global->adj_list.resize(n);
            } else if (token == "A") {
                size_t src, dst; double prob, cost;
                iss >> src >> dst >> prob >> cost;
                if (src < _global->num_states)
                    _global->adj_list[src].push_back({dst, prob, cost});
            }
        }
    }
};

// Helper: run full probability-estimation MDP, returns P(cost <= C)
static double run_mdp_prob(const std::string& param_file, size_t budget = 1000) {
    auto global = std::make_shared<mdp_prob_global_t>();
    mdp_prob_input_parser_t parser(param_file, global);
    parser.parse();
    global->set_montecarlo_budget(budget);

    auto system = system_t::create(global, "mdp_prob_system");
    auto process = process_t::create("walker");
    process->add_thread(std::make_shared<mdp_prob_thread_t>());
    system->add_process(process);

    auto sim = std::make_shared<mdp_prob_simulator_t>(system);
    auto mc = montecarlo_t::create(sim);
    mc->run();

    return global->get_montecarlo_avg();
}

// Helper: run with explicit global (no file parsing)
static double run_mdp_prob_inline(
    size_t num_states,
    double max_cost,
    const std::vector<std::vector<transition_t>>& adj,
    size_t budget = 5000)
{
    auto global = std::make_shared<mdp_prob_global_t>();
    global->num_states = num_states;
    global->max_allowed_cost = max_cost;
    global->adj_list = adj;
    global->set_montecarlo_budget(budget);

    auto system = system_t::create(global, "inline_prob_system");
    auto process = process_t::create("walker");
    process->add_thread(std::make_shared<mdp_prob_thread_t>());
    system->add_process(process);

    auto sim = std::make_shared<mdp_prob_simulator_t>(system);
    auto mc = montecarlo_t::create(sim);
    mc->run();

    return global->get_montecarlo_avg();
}

// ============================================================================
// SECTION 1: Parsing — "C" key support
// ============================================================================

TEST_CASE("Ex2 Parser: reads C threshold and graph", "[ex2][parser]") {
    auto global = std::make_shared<mdp_prob_global_t>();
    mdp_prob_input_parser_t parser("tests/parameters/params_09_01_ex2_t1.txt", global);
    parser.parse();

    REQUIRE(global->max_allowed_cost == Catch::Approx(150.0));
    REQUIRE(global->num_states == 4);
    REQUIRE(global->adj_list.size() == 4);
    REQUIRE(global->adj_list[0].size() == 2);
    REQUIRE(global->adj_list[0][0].prob == Catch::Approx(0.7));
    REQUIRE(global->adj_list[0][1].prob == Catch::Approx(0.3));
}

TEST_CASE("Ex2 Parser: reads all 5 parameter files correctly", "[ex2][parser]") {
    struct expected_t { double C; size_t N; };
    std::vector<expected_t> expected = {
        {150.0, 4}, {200.0, 5}, {200.0, 5}, {200.0, 5}, {200.0, 5}
    };

    for (int i = 1; i <= 5; ++i) {
        auto global = std::make_shared<mdp_prob_global_t>();
        std::string file = "tests/parameters/params_09_01_ex2_t" + std::to_string(i) + ".txt";
        mdp_prob_input_parser_t parser(file, global);
        parser.parse();

        INFO("Test " << i);
        REQUIRE(global->max_allowed_cost == Catch::Approx(expected[i-1].C));
        REQUIRE(global->num_states == expected[i-1].N);
    }
}

// ============================================================================
// SECTION 2: lambda_parser_t — "C" key binding
// ============================================================================

TEST_CASE("Ex2 lambda_parser: parses C, N, A keys", "[ex2][lambda_parser]") {
    auto global = std::make_shared<mdp_prob_global_t>();

    lambda_parser_t lp("tests/parameters/params_09_01_ex2_t1.txt",
        std::unordered_map<std::string, parser>{
            {"C", [&](std::istringstream& iss) {
                double c; iss >> c;
                global->max_allowed_cost = c;
            }},
            {"N", [&](std::istringstream& iss) {
                size_t n; iss >> n;
                global->num_states = n;
                global->adj_list.resize(n);
            }},
            {"A", [&](std::istringstream& iss) {
                size_t src, dst; double prob, cost;
                iss >> src >> dst >> prob >> cost;
                if (src < global->num_states)
                    global->adj_list[src].push_back({dst, prob, cost});
            }}
        });
    lp.parse();

    REQUIRE(global->max_allowed_cost == Catch::Approx(150.0));
    REQUIRE(global->num_states == 4);
    REQUIRE(global->adj_list[0].size() == 2);
    REQUIRE(global->adj_list[1].size() == 1);
    REQUIRE(global->adj_list[2].size() == 1);
    REQUIRE(global->adj_list[3].size() == 1);
}

TEST_CASE("Ex2 lambda_parser: parser equivalence with input_parser", "[ex2][lambda_parser]") {
    // Parse same file with both parsers, verify identical results
    auto g1 = std::make_shared<mdp_prob_global_t>();
    mdp_prob_input_parser_t mp("tests/parameters/params_09_01_ex2_t5.txt", g1);
    mp.parse();

    auto g2 = std::make_shared<mdp_prob_global_t>();
    lambda_parser_t lp("tests/parameters/params_09_01_ex2_t5.txt",
        std::unordered_map<std::string, parser>{
            {"C", [&](std::istringstream& iss) { double c; iss >> c; g2->max_allowed_cost = c; }},
            {"N", [&](std::istringstream& iss) { size_t n; iss >> n; g2->num_states = n; g2->adj_list.resize(n); }},
            {"A", [&](std::istringstream& iss) {
                size_t s, d; double p, c;
                iss >> s >> d >> p >> c;
                if (s < g2->num_states) g2->adj_list[s].push_back({d, p, c});
            }}
        });
    lp.parse();

    REQUIRE(g1->max_allowed_cost == Catch::Approx(g2->max_allowed_cost));
    REQUIRE(g1->num_states == g2->num_states);
    for (size_t i = 0; i < g1->num_states; ++i) {
        REQUIRE(g1->adj_list[i].size() == g2->adj_list[i].size());
        for (size_t j = 0; j < g1->adj_list[i].size(); ++j) {
            REQUIRE(g1->adj_list[i][j].target == g2->adj_list[i][j].target);
            REQUIRE(g1->adj_list[i][j].prob == Catch::Approx(g2->adj_list[i][j].prob));
            REQUIRE(g1->adj_list[i][j].cost == Catch::Approx(g2->adj_list[i][j].cost));
        }
    }
}

// ============================================================================
// SECTION 3: Monte Carlo — indicator-variable probability estimation
// ============================================================================

TEST_CASE("Ex2 Montecarlo: deterministic success (cost < C)", "[ex2][montecarlo]") {
    // Single path 0→1→2 with cost 30+20=50, threshold C=100
    // P(cost <= 100) = 1.0 always
    std::vector<std::vector<transition_t>> adj(3);
    adj[0] = {{1, 1.0, 30.0}};
    adj[1] = {{2, 1.0, 20.0}};
    adj[2] = {{2, 1.0, 0.0}};

    double prob = run_mdp_prob_inline(3, 100.0, adj, 500);
    REQUIRE(prob == Catch::Approx(1.0).margin(0.001));
}

TEST_CASE("Ex2 Montecarlo: deterministic failure (cost > C)", "[ex2][montecarlo]") {
    // Single path 0→1→2 with cost 80+80=160, threshold C=100
    // P(cost <= 100) = 0.0 always
    std::vector<std::vector<transition_t>> adj(3);
    adj[0] = {{1, 1.0, 80.0}};
    adj[1] = {{2, 1.0, 80.0}};
    adj[2] = {{2, 1.0, 0.0}};

    double prob = run_mdp_prob_inline(3, 100.0, adj, 500);
    REQUIRE(prob == Catch::Approx(0.0).margin(0.001));
}

TEST_CASE("Ex2 Montecarlo: exact boundary (cost == C)", "[ex2][montecarlo]") {
    // Single path cost = exactly 100, threshold C = 100
    // P(cost <= 100) = 1.0 (uses <=)
    std::vector<std::vector<transition_t>> adj(3);
    adj[0] = {{1, 1.0, 60.0}};
    adj[1] = {{2, 1.0, 40.0}};
    adj[2] = {{2, 1.0, 0.0}};

    double prob = run_mdp_prob_inline(3, 100.0, adj, 500);
    REQUIRE(prob == Catch::Approx(1.0).margin(0.001));
}

TEST_CASE("Ex2 Montecarlo: 50/50 probability estimation", "[ex2][montecarlo]") {
    // 0→1 (p=0.5, c=10) → 1→3: total 20 ≤ 70 → success
    // 0→2 (p=0.5, c=60) → 2→3: total 120 > 70 → failure
    // P(success) = 0.5
    std::vector<std::vector<transition_t>> adj(4);
    adj[0] = {{1, 0.5, 10.0}, {2, 0.5, 60.0}};
    adj[1] = {{3, 1.0, 10.0}};
    adj[2] = {{3, 1.0, 60.0}};
    adj[3] = {{3, 1.0, 0.0}};

    double prob = run_mdp_prob_inline(4, 70.0, adj, 10000);
    REQUIRE(prob == Catch::Approx(0.5).margin(0.03));
}

TEST_CASE("Ex2 Montecarlo: 70/30 split matches expected probability", "[ex2][montecarlo]") {
    // Mirrors graded Test 1 inline:
    // 0→1 (p=0.7, c=100), 0→2 (p=0.3, c=50)
    // 1→3 (c=100): cost=200 > 150 → fail
    // 2→3 (c=100): cost=150 ≤ 150 → success
    // P(success) = 0.3
    std::vector<std::vector<transition_t>> adj(4);
    adj[0] = {{1, 0.7, 100.0}, {2, 0.3, 50.0}};
    adj[1] = {{3, 1.0, 100.0}};
    adj[2] = {{3, 1.0, 100.0}};
    adj[3] = {{3, 1.0, 0.0}};

    double prob = run_mdp_prob_inline(4, 150.0, adj, 10000);
    REQUIRE(prob == Catch::Approx(0.3).margin(0.03));
}

TEST_CASE("Ex2 Montecarlo: all paths succeed → P=1", "[ex2][montecarlo]") {
    std::vector<std::vector<transition_t>> adj(4);
    adj[0] = {{1, 0.4, 10.0}, {2, 0.6, 20.0}};
    adj[1] = {{3, 1.0, 10.0}};
    adj[2] = {{3, 1.0, 10.0}};
    adj[3] = {{3, 1.0, 0.0}};

    double prob = run_mdp_prob_inline(4, 1000.0, adj, 1000);
    REQUIRE(prob == Catch::Approx(1.0).margin(0.001));
}

TEST_CASE("Ex2 Montecarlo: all paths fail → P=0", "[ex2][montecarlo]") {
    std::vector<std::vector<transition_t>> adj(4);
    adj[0] = {{1, 0.5, 100.0}, {2, 0.5, 200.0}};
    adj[1] = {{3, 1.0, 100.0}};
    adj[2] = {{3, 1.0, 100.0}};
    adj[3] = {{3, 1.0, 0.0}};

    double prob = run_mdp_prob_inline(4, 50.0, adj, 1000);
    REQUIRE(prob == Catch::Approx(0.0).margin(0.001));
}

TEST_CASE("Ex2 Montecarlo: three-way branching probability", "[ex2][montecarlo]") {
    // 0 → 1 (p=0.2, c=10), 0 → 2 (p=0.3, c=50), 0 → 3 (p=0.5, c=200)
    // 1 → 4 (c=10): cost=20 ≤ 100 → success
    // 2 → 4 (c=10): cost=60 ≤ 100 → success
    // 3 → 4 (c=10): cost=210 > 100 → fail
    // P(success) = 0.2 + 0.3 = 0.5
    std::vector<std::vector<transition_t>> adj(5);
    adj[0] = {{1, 0.2, 10.0}, {2, 0.3, 50.0}, {3, 0.5, 200.0}};
    adj[1] = {{4, 1.0, 10.0}};
    adj[2] = {{4, 1.0, 10.0}};
    adj[3] = {{4, 1.0, 10.0}};
    adj[4] = {{4, 1.0, 0.0}};

    double prob = run_mdp_prob_inline(5, 100.0, adj, 10000);
    REQUIRE(prob == Catch::Approx(0.5).margin(0.03));
}

// ============================================================================
// SECTION 4: Monte Carlo — budget sensitivity for probability estimation
// ============================================================================

TEST_CASE("Ex2 Montecarlo: budget=1 gives 0 or 1", "[ex2][montecarlo]") {
    std::vector<std::vector<transition_t>> adj(4);
    adj[0] = {{1, 0.5, 10.0}, {2, 0.5, 200.0}};
    adj[1] = {{3, 1.0, 10.0}};
    adj[2] = {{3, 1.0, 10.0}};
    adj[3] = {{3, 1.0, 0.0}};

    double prob = run_mdp_prob_inline(4, 100.0, adj, 1);
    bool valid = (std::abs(prob - 0.0) < 0.001) || (std::abs(prob - 1.0) < 0.001);
    REQUIRE(valid);
}

TEST_CASE("Ex2 Montecarlo: high budget converges", "[ex2][montecarlo][convergence]") {
    // P(success) = 0.3 (Test 1 scenario)
    std::vector<std::vector<transition_t>> adj(4);
    adj[0] = {{1, 0.7, 100.0}, {2, 0.3, 50.0}};
    adj[1] = {{3, 1.0, 100.0}};
    adj[2] = {{3, 1.0, 100.0}};
    adj[3] = {{3, 1.0, 0.0}};

    for (int trial = 0; trial < 5; ++trial) {
        double prob = run_mdp_prob_inline(4, 150.0, adj, 10000);
        INFO("Trial " << trial << ": P = " << prob);
        REQUIRE(prob == Catch::Approx(0.3).margin(0.03));
    }
}

TEST_CASE("Ex2 Montecarlo: varying threshold C changes probability", "[ex2][montecarlo]") {
    // Paths: 0→1→3 cost=200, 0→2→3 cost=150
    std::vector<std::vector<transition_t>> adj(4);
    adj[0] = {{1, 0.7, 100.0}, {2, 0.3, 50.0}};
    adj[1] = {{3, 1.0, 100.0}};
    adj[2] = {{3, 1.0, 100.0}};
    adj[3] = {{3, 1.0, 0.0}};

    SECTION("C=100: no path has cost ≤ 100 → P=0") {
        double p = run_mdp_prob_inline(4, 100.0, adj, 5000);
        REQUIRE(p == Catch::Approx(0.0).margin(0.001));
    }

    SECTION("C=150: only 0→2→3 (cost=150) succeeds → P=0.3") {
        double p = run_mdp_prob_inline(4, 150.0, adj, 5000);
        REQUIRE(p == Catch::Approx(0.3).margin(0.03));
    }

    SECTION("C=200: both paths succeed → P=1.0") {
        double p = run_mdp_prob_inline(4, 200.0, adj, 5000);
        REQUIRE(p == Catch::Approx(1.0).margin(0.001));
    }

    SECTION("C=500: all paths under budget → P=1.0") {
        double p = run_mdp_prob_inline(4, 500.0, adj, 1000);
        REQUIRE(p == Catch::Approx(1.0).margin(0.001));
    }
}

// ============================================================================
// SECTION 5: Montecarlo with loops (non-absorbing intermediate states)
// ============================================================================

TEST_CASE("Ex2 Montecarlo: loop-back graph probability", "[ex2][montecarlo][loops]") {
    // 0→1 (p=1, c=100)
    // 1→2 (p=0.5, c=100), 1→0 (p=0.5, c=10)  ← loop back
    // 2 terminal
    // On attempt k (0-indexed): cost = (100+10)*k + 100 + 100 = 200 + 110*k
    // For C=200: only k=0 (cost=200), P = 0.5
    std::vector<std::vector<transition_t>> adj(3);
    adj[0] = {{1, 1.0, 100.0}};
    adj[1] = {{2, 0.5, 100.0}, {0, 0.5, 10.0}};
    adj[2] = {{2, 1.0, 0.0}};

    double prob = run_mdp_prob_inline(3, 200.0, adj, 10000);
    REQUIRE(prob == Catch::Approx(0.5).margin(0.03));
}

TEST_CASE("Ex2 Montecarlo: loop allows multiple attempts", "[ex2][montecarlo][loops]") {
    // Same graph but higher threshold allows loop-back paths
    // cost for k loop-backs before success: 200 + 110*k
    // For C=500: success if k ≤ floor((500-200)/110) = floor(2.727) = 2
    // P(k=0) = 0.5, P(k=1) = 0.25, P(k=2) = 0.125
    // P(cost ≤ 500) = 0.5 + 0.25 + 0.125 = 0.875
    std::vector<std::vector<transition_t>> adj(3);
    adj[0] = {{1, 1.0, 100.0}};
    adj[1] = {{2, 0.5, 100.0}, {0, 0.5, 10.0}};
    adj[2] = {{2, 1.0, 0.0}};

    double prob = run_mdp_prob_inline(3, 500.0, adj, 10000);
    REQUIRE(prob == Catch::Approx(0.875).margin(0.03));
}

// ============================================================================
// SECTION 6: montecarlo_t — running average correctness
// ============================================================================

TEST_CASE("Ex2 Montecarlo: running average of all-success is 1.0", "[ex2][montecarlo][average]") {
    std::vector<std::vector<transition_t>> adj(2);
    adj[0] = {{1, 1.0, 10.0}};
    adj[1] = {{1, 1.0, 0.0}};

    auto global = std::make_shared<mdp_prob_global_t>();
    global->num_states = 2;
    global->max_allowed_cost = 100.0;
    global->adj_list = adj;
    global->set_montecarlo_budget(100);

    auto system = system_t::create(global, "avg_test");
    auto process = process_t::create("walker");
    process->add_thread(std::make_shared<mdp_prob_thread_t>());
    system->add_process(process);

    auto sim = std::make_shared<mdp_prob_simulator_t>(system);
    auto mc = montecarlo_t::create(sim);
    mc->run();

    REQUIRE(global->get_montecarlo_avg() == Catch::Approx(1.0).margin(0.001));
}

TEST_CASE("Ex2 Montecarlo: avg of all-failures is 0", "[ex2][montecarlo][average]") {
    std::vector<std::vector<transition_t>> adj(2);
    adj[0] = {{1, 1.0, 500.0}};
    adj[1] = {{1, 1.0, 0.0}};

    auto global = std::make_shared<mdp_prob_global_t>();
    global->num_states = 2;
    global->max_allowed_cost = 100.0;
    global->adj_list = adj;
    global->set_montecarlo_budget(100);

    auto system = system_t::create(global, "avg_fail_test");
    auto process = process_t::create("walker");
    process->add_thread(std::make_shared<mdp_prob_thread_t>());
    system->add_process(process);

    auto sim = std::make_shared<mdp_prob_simulator_t>(system);
    auto mc = montecarlo_t::create(sim);
    mc->run();

    REQUIRE(global->get_montecarlo_avg() == Catch::Approx(0.0).margin(0.001));
}

// ============================================================================
// SECTION 7: montecarlo_t::create template factory
// ============================================================================

TEST_CASE("Ex2 Montecarlo: template create<SIM> factory", "[ex2][montecarlo][factory]") {
    std::vector<std::vector<transition_t>> adj(2);
    adj[0] = {{1, 1.0, 10.0}};
    adj[1] = {{1, 1.0, 0.0}};

    auto global = std::make_shared<mdp_prob_global_t>();
    global->num_states = 2;
    global->max_allowed_cost = 100.0;
    global->adj_list = adj;
    global->set_montecarlo_budget(50);

    auto system = system_t::create(global, "factory_test");
    auto process = process_t::create("walker");
    process->add_thread(std::make_shared<mdp_prob_thread_t>());
    system->add_process(process);

    auto mc = montecarlo_t::create<mdp_prob_simulator_t>(system);
    mc->run();

    REQUIRE(global->get_montecarlo_avg() == Catch::Approx(1.0).margin(0.001));
}

// ============================================================================
// SECTION 8: system_t::init resets between montecarlo runs
// ============================================================================

TEST_CASE("Ex2 system init: properly resets state between runs", "[ex2][system][montecarlo]") {
    std::vector<std::vector<transition_t>> adj(4);
    adj[0] = {{1, 0.7, 100.0}, {2, 0.3, 50.0}};
    adj[1] = {{3, 1.0, 100.0}};
    adj[2] = {{3, 1.0, 100.0}};
    adj[3] = {{3, 1.0, 0.0}};

    auto global = std::make_shared<mdp_prob_global_t>();
    global->num_states = 4;
    global->max_allowed_cost = 150.0;
    global->adj_list = adj;
    global->set_montecarlo_budget(5000);

    auto system = system_t::create(global, "reset_test");
    auto process = process_t::create("walker");
    process->add_thread(std::make_shared<mdp_prob_thread_t>());
    system->add_process(process);

    auto sim = std::make_shared<mdp_prob_simulator_t>(system);
    auto mc = montecarlo_t::create(sim);

    mc->run();
    double result1 = global->get_montecarlo_avg();

    mc->run();
    double result2 = global->get_montecarlo_avg();

    // Both should be near 0.3
    REQUIRE(result1 == Catch::Approx(0.3).margin(0.04));
    REQUIRE(result2 == Catch::Approx(0.3).margin(0.04));
}

// ============================================================================
// SECTION 9: Probability sum validation for ex2 parameter files
// ============================================================================

TEST_CASE("Ex2 validation: probability sums equal 1 for all states", "[ex2][validation]") {
    for (int test_id = 1; test_id <= 5; ++test_id) {
        std::string file = "tests/parameters/params_09_01_ex2_t" + std::to_string(test_id) + ".txt";
        auto global = std::make_shared<mdp_prob_global_t>();
        mdp_prob_input_parser_t parser(file, global);
        parser.parse();

        for (size_t s = 0; s < global->num_states; ++s) {
            if (!global->adj_list[s].empty()) {
                double total = 0;
                for (const auto& t : global->adj_list[s])
                    total += t.prob;
                INFO("Test " << test_id << ", state " << s << ": prob sum = " << total);
                REQUIRE(total == Catch::Approx(1.0).margin(1e-9));
            }
        }
    }
}

// ============================================================================
// SECTION 10: Graded Test Scenarios
// ============================================================================

TEST_CASE("Ex2 Graded Test 1: P(cost <= 150) ≈ 0.3", "[ex2][graded]") {
    // Analytical:
    // Path 0→1→3: cost = 100+100 = 200 > 150 → fail (prob 0.7)
    // Path 0→2→3: cost = 50+100  = 150 ≤ 150 → success (prob 0.3)
    // P(success) = 0.3
    double result = run_mdp_prob("tests/parameters/params_09_01_ex2_t1.txt", 10000);
    INFO("Test 1 result: P = " << result << ", expected ~0.3");
    REQUIRE(result == Catch::Approx(0.3).margin(0.03));
}

TEST_CASE("Ex2 Graded Test 5: P(cost <= 200) ≈ 0.15", "[ex2][graded]") {
    // Test 5 has non-absorbing state 3 (A 3 3 0.8 70, A 3 1 0.2 10)
    // and state 4 is reachable via 1→4. Complex loop structure.
    // Grader results: ~0.145, ~0.158 → expected ≈ 0.15
    double result = run_mdp_prob("tests/parameters/params_09_01_ex2_t5.txt", 10000);
    INFO("Test 5 result: P = " << result << ", expected ~0.15");
    REQUIRE(result == Catch::Approx(0.15).margin(0.04));
}

// ============================================================================
// SECTION 11: End-to-end with output file
// ============================================================================

TEST_CASE("Ex2 E2E: full pipeline from parsing to output", "[ex2][e2e]") {
    const std::string result_file = "tests/_tmp_ex2_e2e.txt";

    auto global = std::make_shared<mdp_prob_global_t>();
    mdp_prob_input_parser_t parser("tests/parameters/params_09_01_ex2_t1.txt", global);
    parser.parse();
    global->set_montecarlo_budget(5000);

    auto system = system_t::create(global, "e2e_prob_system");
    auto process = process_t::create("walker");
    process->add_thread(std::make_shared<mdp_prob_thread_t>());
    system->add_process(process);

    auto sim = std::make_shared<mdp_prob_simulator_t>(system);
    auto mc = montecarlo_t::create(sim);
    mc->run();

    {
        output_writer_t writer(result_file);
        writer.write_line("2025-01-09-Test-E2E-0000000");
        writer << "P " << global->get_montecarlo_avg() << std::endl;
    }

    // Validate file
    std::ifstream in(result_file);
    REQUIRE(in.is_open());
    std::string line1, line2;
    std::getline(in, line1);
    std::getline(in, line2);

    REQUIRE(line1 == "2025-01-09-Test-E2E-0000000");
    REQUIRE(line2.substr(0, 2) == "P ");
    double prob = std::stod(line2.substr(2));
    REQUIRE(prob == Catch::Approx(0.3).margin(0.03));

    in.close();
    std::filesystem::remove(result_file);
}

// ============================================================================
// SECTION 12: Edge cases
// ============================================================================

TEST_CASE("Ex2 Edge: zero-cost path always succeeds", "[ex2][edge]") {
    std::vector<std::vector<transition_t>> adj(2);
    adj[0] = {{1, 1.0, 0.0}};
    adj[1] = {{1, 1.0, 0.0}};

    double prob = run_mdp_prob_inline(2, 0.0, adj, 500);
    REQUIRE(prob == Catch::Approx(1.0).margin(0.001));
}

TEST_CASE("Ex2 Edge: tiny C makes deterministic path fail", "[ex2][edge]") {
    std::vector<std::vector<transition_t>> adj(2);
    adj[0] = {{1, 1.0, 100.0}};
    adj[1] = {{1, 1.0, 0.0}};

    double prob = run_mdp_prob_inline(2, 1.0, adj, 500);
    REQUIRE(prob == Catch::Approx(0.0).margin(0.001));
}

TEST_CASE("Ex2 Edge: long chain within budget", "[ex2][edge]") {
    // 0→1→2→3→4→5 each costing 10, C=50
    // Total cost = 50 ≤ 50, P = 1.0
    std::vector<std::vector<transition_t>> adj(6);
    for (int i = 0; i < 5; ++i)
        adj[i] = {{static_cast<size_t>(i + 1), 1.0, 10.0}};
    adj[5] = {{5, 1.0, 0.0}};

    double prob = run_mdp_prob_inline(6, 50.0, adj, 500);
    REQUIRE(prob == Catch::Approx(1.0).margin(0.001));
}

TEST_CASE("Ex2 Edge: long chain just over budget", "[ex2][edge]") {
    // 0→1→2→3→4→5 each costing 10, C=49
    // Total cost = 50 > 49, P = 0.0
    std::vector<std::vector<transition_t>> adj(6);
    for (int i = 0; i < 5; ++i)
        adj[i] = {{static_cast<size_t>(i + 1), 1.0, 10.0}};
    adj[5] = {{5, 1.0, 0.0}};

    double prob = run_mdp_prob_inline(6, 49.0, adj, 500);
    REQUIRE(prob == Catch::Approx(0.0).margin(0.001));
}

TEST_CASE("Ex2 Edge: probability monotone in C", "[ex2][edge]") {
    // P(cost <= C) should be non-decreasing as C increases
    std::vector<std::vector<transition_t>> adj(4);
    adj[0] = {{1, 0.7, 100.0}, {2, 0.3, 50.0}};
    adj[1] = {{3, 1.0, 100.0}};
    adj[2] = {{3, 1.0, 100.0}};
    adj[3] = {{3, 1.0, 0.0}};

    double p_low = run_mdp_prob_inline(4, 100.0, adj, 5000);
    double p_mid = run_mdp_prob_inline(4, 150.0, adj, 5000);
    double p_high = run_mdp_prob_inline(4, 300.0, adj, 5000);

    REQUIRE(p_low <= p_mid + 0.01);
    REQUIRE(p_mid <= p_high + 0.01);
}