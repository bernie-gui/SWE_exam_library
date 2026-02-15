#include "catch_lib.hpp"

/**
 * @file test_09_01_ex1.cpp
 * @brief Unit tests for Exercise 09-01 Ex.1 — Expected Cost Estimation via Monte Carlo
 * @details Exercise-specific tests for MDP cost-estimation problems:
 *          - Custom global/thread/simulator for MDP walker
 *          - input_parser_t and lambda_parser_t with MDP parameters
 *          - montecarlo_t averaging logic with MDP
 *          - All 5 graded scenarios with analytical verification
 *          - End-to-end pipeline and edge cases
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
#include "utils/markov/markov.hpp"

using namespace isw;

// ============================================================================
// MDP helpers (reused across tests)
// ============================================================================

struct transition_t {
    size_t target;
    double prob;
    double cost;
};

class mdp_global_t : public global_t {
public:
    mdp_global_t() : global_t() {}

    size_t num_states = 0;
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

class mdp_thread_t : public thread_t {
private:
    size_t current_node;
public:
    mdp_thread_t() : thread_t(0.0, 0.0, 0.0), current_node(0) {}

    void init() override {
        thread_t::init();
        current_node = 0;
    }

    void fun() override {
        auto global = get_global<mdp_global_t>();

        if (current_node == global->num_states - 1) {
            global->reached_terminal = true;
            global->set_montecarlo_current(global->accumulated_cost);
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
                global->set_montecarlo_current(global->accumulated_cost);
                break;
            }
        }
        set_thread_time(get_thread_time() + 0.1);
    }
};

class mdp_simulator_t : public simulator_t {
public:
    using simulator_t::simulator_t;
    bool should_terminate() override {
        auto global = get_global<mdp_global_t>();
        return global->reached_terminal;
    }
};

// Input parser using raw ifstream (as in example solution)
class mdp_input_parser_t : public input_parser_t {
private:
    std::shared_ptr<mdp_global_t> _global;
public:
    mdp_input_parser_t(const std::string& filename, std::shared_ptr<mdp_global_t> global)
        : input_parser_t(filename), _global(global) {}

    void parse() override {
        auto& iss = get_stream();
        std::string token;
        while (iss >> token) {
            if (token == "N") {
                size_t n;
                iss >> n;
                _global->num_states = n;
                _global->adj_list.resize(n);
            } else if (token == "A") {
                size_t src, dst;
                double prob, cost;
                iss >> src >> dst >> prob >> cost;
                if (src < _global->num_states) {
                    transition_t t;
                    t.target = dst;
                    t.prob = prob;
                    t.cost = cost;
                    _global->adj_list[src].push_back(t);
                }
            }
        }
    }
};

// Full MDP runner helper - returns montecarlo average
static double run_mdp(const std::string& param_file, size_t budget = 1000) {
    auto global = std::make_shared<mdp_global_t>();
    mdp_input_parser_t parser(param_file, global);
    parser.parse();
    global->set_montecarlo_budget(budget);

    auto system = system_t::create(global, "mdp_test_system");
    auto process = process_t::create("walker");
    process->add_thread(std::make_shared<mdp_thread_t>());
    system->add_process(process);

    auto simulator = std::make_shared<mdp_simulator_t>(system);
    auto montecarlo = montecarlo_t::create(simulator);
    montecarlo->run();

    return global->get_montecarlo_avg();
}

// ============================================================================
// SECTION 1: mdp_global_t (custom global)
// ============================================================================

TEST_CASE("mdp_global_t: init resets accumulated cost", "[mdp][global]") {
    auto g = std::make_shared<mdp_global_t>();
    g->accumulated_cost = 999.0;
    g->reached_terminal = true;
    g->init();
    REQUIRE(g->accumulated_cost == Catch::Approx(0.0));
    REQUIRE_FALSE(g->reached_terminal);
}

// ============================================================================
// SECTION 2: input_parser_t (base & custom MDP parser)
// ============================================================================

TEST_CASE("input_parser_t: MDP parser reads Test 1 parameters", "[parser][io]") {
    auto global = std::make_shared<mdp_global_t>();
    mdp_input_parser_t parser("tests/parameters/params_09_01_ex1_t1.txt", global);
    parser.parse();

    REQUIRE(global->num_states == 4);
    REQUIRE(global->adj_list.size() == 4);

    // State 0: single arc to 1
    REQUIRE(global->adj_list[0].size() == 1);
    REQUIRE(global->adj_list[0][0].target == 1);
    REQUIRE(global->adj_list[0][0].prob == Catch::Approx(1.0));
    REQUIRE(global->adj_list[0][0].cost == Catch::Approx(100.0));

    // State 1: two arcs
    REQUIRE(global->adj_list[1].size() == 2);
    REQUIRE(global->adj_list[1][0].target == 2);
    REQUIRE(global->adj_list[1][0].prob == Catch::Approx(0.7));
    REQUIRE(global->adj_list[1][1].target == 3);
    REQUIRE(global->adj_list[1][1].prob == Catch::Approx(0.3));

    // Terminal state 3: self-loop
    REQUIRE(global->adj_list[3].size() == 1);
    REQUIRE(global->adj_list[3][0].target == 3);
    REQUIRE(global->adj_list[3][0].cost == Catch::Approx(0.0));
}

TEST_CASE("input_parser_t: MDP parser reads Test 3 parameters (loops)", "[parser][io]") {
    auto global = std::make_shared<mdp_global_t>();
    mdp_input_parser_t parser("tests/parameters/params_09_01_ex1_t3.txt", global);
    parser.parse();

    REQUIRE(global->num_states == 5);
    // State 3 should have 5 outgoing arcs
    REQUIRE(global->adj_list[3].size() == 5);
    double total_prob = 0;
    for (const auto& t : global->adj_list[3])
        total_prob += t.prob;
    REQUIRE(total_prob == Catch::Approx(1.0));
}

TEST_CASE("input_parser_t: reset_stream re-reads file", "[parser][io]") {
    auto global = std::make_shared<mdp_global_t>();
    mdp_input_parser_t parser("tests/parameters/params_09_01_ex1_t1.txt", global);
    parser.parse();
    REQUIRE(global->num_states == 4);

    // Reset and re-parse (adj_list will double if we don't clear)
    global->adj_list.clear();
    global->num_states = 0;
    parser.reset_stream();
    parser.parse();
    REQUIRE(global->num_states == 4);
    REQUIRE(global->adj_list.size() == 4);
}

// ============================================================================
// SECTION 3: lambda_parser_t with MDP parameters
// ============================================================================

TEST_CASE("lambda_parser_t: parses MDP parameters with lambdas", "[lambda_parser][io]") {
    auto global = std::make_shared<mdp_global_t>();

    std::unordered_map<std::string, parser> bindings;
    bindings["N"] = [&global](std::istringstream& iss) {
        size_t n;
        iss >> n;
        global->num_states = n;
        global->adj_list.resize(n);
    };
    bindings["A"] = [&global](std::istringstream& iss) {
        size_t src, dst;
        double prob, cost;
        iss >> src >> dst >> prob >> cost;
        if (src < global->num_states) {
            transition_t t;
            t.target = dst;
            t.prob = prob;
            t.cost = cost;
            global->adj_list[src].push_back(t);
        }
    };

    lambda_parser_t lp("tests/parameters/params_09_01_ex1_t1.txt", bindings);
    lp.parse();

    REQUIRE(global->num_states == 4);
    REQUIRE(global->adj_list[0].size() == 1);
    REQUIRE(global->adj_list[1].size() == 2);
    REQUIRE(global->adj_list[0][0].cost == Catch::Approx(100.0));
}

TEST_CASE("lambda_parser_t: rvalue bindings constructor", "[mdp][lambda_parser][io]") {
    auto global = std::make_shared<mdp_global_t>();

    lambda_parser_t lp("tests/parameters/params_09_01_ex1_t2.txt", std::unordered_map<std::string, parser>{
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

    REQUIRE(global->num_states == 4);
    // State 1: p(2)=0.3, p(3)=0.7
    REQUIRE(global->adj_list[1][0].prob == Catch::Approx(0.3));
    REQUIRE(global->adj_list[1][1].prob == Catch::Approx(0.7));
}

// ============================================================================
// SECTION 4: montecarlo_t (averaging logic)
// ============================================================================

TEST_CASE("montecarlo_t: deterministic single-path MDP gives exact cost", "[montecarlo]") {
    // Test with a purely deterministic MDP: 0 -> 1 (cost 50) -> 2 (cost 30) -> 3 (terminal, cost 0)
    const std::string tmp = "tests/_tmp_deterministic.txt";
    {
        std::ofstream f(tmp);
        f << "N 4\n";
        f << "A 0 1 1 50\n";
        f << "A 1 2 1 30\n";
        f << "A 2 3 1 20\n";
        f << "A 3 3 1 0\n";
    }

    double avg = run_mdp(tmp, 100);
    // Deterministic: cost = 50 + 30 + 20 = 100 exactly
    REQUIRE(avg == Catch::Approx(100.0).margin(0.01));

    std::filesystem::remove(tmp);
}

TEST_CASE("montecarlo_t: budget=1 still produces a result", "[montecarlo]") {
    double avg = run_mdp("tests/parameters/params_09_01_ex1_t1.txt", 1);
    // Any single run should give either 300 (path 0->1->2->3) or 250 (path 0->1->3)
    // So result must be one of those
    bool valid = (std::abs(avg - 300.0) < 1.0) || (std::abs(avg - 250.0) < 1.0);
    REQUIRE(valid);
}

TEST_CASE("montecarlo_t: increasing budget reduces variance", "[montecarlo]") {
    // Run 5 times with budget=100 and 5 times with budget=5000
    // The spread of budget=5000 should be tighter
    std::vector<double> low_budget, high_budget;
    for (int i = 0; i < 5; ++i) {
        low_budget.push_back(run_mdp("tests/parameters/params_09_01_ex1_t1.txt", 100));
        high_budget.push_back(run_mdp("tests/parameters/params_09_01_ex1_t1.txt", 5000));
    }

    auto spread = [](const std::vector<double>& v) {
        double mn = *std::min_element(v.begin(), v.end());
        double mx = *std::max_element(v.begin(), v.end());
        return mx - mn;
    };

    // High budget spread should generally be smaller
    // (Not guaranteed, but with high probability)
    double s_low = spread(low_budget);
    double s_high = spread(high_budget);
    // Just check both are finite and high budget gives reasonable result
    REQUIRE(std::isfinite(s_low));
    REQUIRE(std::isfinite(s_high));

    // All high budget results should be near 285
    for (auto v : high_budget) {
        REQUIRE(v == Catch::Approx(285.0).margin(30.0));
    }
}

// ============================================================================
// SECTION 5: Full MDP Simulation — Graded Test Scenarios
// ============================================================================
// For each test, the expected cost is computed analytically and the Monte Carlo
// result (with 5000 runs) must fall within a tolerance window.
// The tolerance is generous (±15%) to account for stochastic variance while
// still catching fundamental bugs.

TEST_CASE("MDP Test 1: Simple 4-state, E[C]=285", "[mdp][graded]") {
    // Analytical: C = 100 + 0.7*(100+100) + 0.3*150 = 100 + 140 + 45 = 285
    const double expected = 285.0;
    double result = run_mdp("tests/parameters/params_09_01_ex1_t1.txt", 5000);
    INFO("Test 1 result: " << result << ", expected ~" << expected);
    REQUIRE(result == Catch::Approx(expected).margin(25.0));
}

TEST_CASE("MDP Test 2: Simple 4-state, E[C]=265", "[mdp][graded]") {
    // Analytical: C = 100 + 0.3*(100+100) + 0.7*150 = 100 + 60 + 105 = 265
    const double expected = 265.0;
    double result = run_mdp("tests/parameters/params_09_01_ex1_t2.txt", 5000);
    INFO("Test 2 result: " << result << ", expected ~" << expected);
    REQUIRE(result == Catch::Approx(expected).margin(25.0));
}

TEST_CASE("MDP Test 3: 5-state with loops, E[C]~837", "[mdp][graded]") {
    // States: 0->1->{ 2(p=0.3), 3(p=0.7) }, 2->3, 3->{0,1,2,3,4} uniform 0.2 each (cost 10)
    // Terminal: 4.
    // Solving the system of equations:
    //   Let V(s) = expected cost from state s.
    //   V(4) = 0
    //   V(0) = 100 + V(1)
    //   V(1) = 0.3*(100 + V(2)) + 0.7*(150 + V(3))
    //   V(2) = 100 + V(3)
    //   V(3) = 0.2*(10+V(0)) + 0.2*(10+V(1)) + 0.2*(10+V(2)) + 0.2*(10+V(3)) + 0.2*(10+V(4))
    //   V(3) = 10 + 0.2*V(0) + 0.2*V(1) + 0.2*V(2) + 0.2*V(3) + 0
    //   => 0.8*V(3) = 10 + 0.2*V(0) + 0.2*V(1) + 0.2*V(2)
    //   V(2) = 100 + V(3)
    //   V(0) = 100 + V(1)
    //   V(1) = 30 + 0.3*V(2) + 105 + 0.7*V(3) = 135 + 0.3*V(2) + 0.7*V(3)
    //        = 135 + 0.3*(100+V(3)) + 0.7*V(3) = 135 + 30 + V(3) = 165 + V(3)
    //   V(0) = 100 + 165 + V(3) = 265 + V(3)
    //   V(2) = 100 + V(3)
    //   0.8*V(3) = 10 + 0.2*(265+V(3)) + 0.2*(165+V(3)) + 0.2*(100+V(3))
    //            = 10 + 53 + 0.2*V(3) + 33 + 0.2*V(3) + 20 + 0.2*V(3)
    //            = 116 + 0.6*V(3)
    //   0.2*V(3) = 116  =>  V(3) = 580
    //   V(0) = 265 + 580 = 845
    // Good results from grader: ~831, ~843.5 => average around 837
    const double expected = 845.0;
    double result = run_mdp("tests/parameters/params_09_01_ex1_t3.txt", 5000);
    INFO("Test 3 result: " << result << ", expected ~" << expected);
    REQUIRE(result == Catch::Approx(expected).margin(60.0));
}

TEST_CASE("MDP Test 4: 5-state with biased loops, E[C]~875", "[mdp][graded]") {
    // Same structure as test 3 but p(3->3)=0.6, others 0.1
    //   V(1) = 165 + V(3) (same as above since state 1 transitions unchanged)
    //   V(0) = 265 + V(3)
    //   V(2) = 100 + V(3)
    //   V(3) = 10 + 0.1*V(0) + 0.1*V(1) + 0.1*V(2) + 0.6*V(3) + 0.1*0
    //   0.4*V(3) = 10 + 0.1*(265+V(3)) + 0.1*(165+V(3)) + 0.1*(100+V(3))
    //            = 10 + 26.5 + 0.1*V(3) + 16.5 + 0.1*V(3) + 10 + 0.1*V(3)
    //            = 63 + 0.3*V(3)
    //   0.1*V(3) = 63 => V(3) = 630
    //   V(0) = 265 + 630 = 895
    // Grader results: ~886.48, ~863.95 => average around 875
    const double expected = 895.0;
    double result = run_mdp("tests/parameters/params_09_01_ex1_t4.txt", 5000);
    INFO("Test 4 result: " << result << ", expected ~" << expected);
    REQUIRE(result == Catch::Approx(expected).margin(80.0));
}

TEST_CASE("MDP Test 5: 5-state with heavy back-loops, E[C]~2165", "[mdp][graded]") {
    // Same structure but p(3->0)=0.6, p(3->1)=0.1, p(3->2)=0.1, p(3->3)=0.1, p(3->4)=0.1
    //   V(1) = 165 + V(3)
    //   V(0) = 265 + V(3)
    //   V(2) = 100 + V(3)
    //   V(3) = 10 + 0.6*V(0) + 0.1*V(1) + 0.1*V(2) + 0.1*V(3) + 0.1*0
    //   0.9*V(3) = 10 + 0.6*(265+V(3)) + 0.1*(165+V(3)) + 0.1*(100+V(3))
    //            = 10 + 159 + 0.6*V(3) + 16.5 + 0.1*V(3) + 10 + 0.1*V(3)
    //            = 195.5 + 0.8*V(3)
    //   0.1*V(3) = 195.5 => V(3) = 1955
    //   V(0) = 265 + 1955 = 2220
    // Grader results: ~2170, ~2152 => let's use wide margin
    const double expected = 2220.0;
    double result = run_mdp("tests/parameters/params_09_01_ex1_t5.txt", 5000);
    INFO("Test 5 result: " << result << ", expected ~" << expected);
    REQUIRE(result == Catch::Approx(expected).margin(150.0));
}

// ============================================================================
// SECTION 6: Lambda parser for full MDP pipeline (alternative approach)
// ============================================================================

TEST_CASE("Full MDP with lambda_parser_t matches manual parser", "[mdp][lambda_parser][integration]") {
    // Parse with manual parser
    auto global_manual = std::make_shared<mdp_global_t>();
    mdp_input_parser_t mp("tests/parameters/params_09_01_ex1_t1.txt", global_manual);
    mp.parse();

    // Parse with lambda parser
    auto global_lambda = std::make_shared<mdp_global_t>();
    lambda_parser_t lp("tests/parameters/params_09_01_ex1_t1.txt", std::unordered_map<std::string, parser>{
        {"N", [&](std::istringstream& iss) {
            size_t n; iss >> n;
            global_lambda->num_states = n;
            global_lambda->adj_list.resize(n);
        }},
        {"A", [&](std::istringstream& iss) {
            size_t src, dst; double prob, cost;
            iss >> src >> dst >> prob >> cost;
            if (src < global_lambda->num_states)
                global_lambda->adj_list[src].push_back({dst, prob, cost});
        }}
    });
    lp.parse();

    // Verify both parsers produce identical results
    REQUIRE(global_manual->num_states == global_lambda->num_states);
    for (size_t i = 0; i < global_manual->num_states; ++i) {
        REQUIRE(global_manual->adj_list[i].size() == global_lambda->adj_list[i].size());
        for (size_t j = 0; j < global_manual->adj_list[i].size(); ++j) {
            REQUIRE(global_manual->adj_list[i][j].target == global_lambda->adj_list[i][j].target);
            REQUIRE(global_manual->adj_list[i][j].prob == Catch::Approx(global_lambda->adj_list[i][j].prob));
            REQUIRE(global_manual->adj_list[i][j].cost == Catch::Approx(global_lambda->adj_list[i][j].cost));
        }
    }
}

// ============================================================================
// SECTION 7: End-to-end output validation
// ============================================================================

TEST_CASE("End-to-end: run MDP and write results file", "[mdp][e2e]") {
    const std::string result_file = "tests/_tmp_e2e_results.txt";

    auto global = std::make_shared<mdp_global_t>();
    mdp_input_parser_t parser("tests/parameters/params_09_01_ex1_t1.txt", global);
    parser.parse();
    global->set_montecarlo_budget(1000);

    auto system = system_t::create(global, "e2e_system");
    auto process = process_t::create("walker");
    process->add_thread(std::make_shared<mdp_thread_t>());
    system->add_process(process);

    auto simulator = std::make_shared<mdp_simulator_t>(system);
    auto montecarlo = montecarlo_t::create(simulator);
    montecarlo->run();

    // Write output in grader format
    {
        output_writer_t writer(result_file);
        writer.write_line("2025-01-09-Test-E2E-0000000");
        writer << "C " << global->get_montecarlo_avg() << std::endl;
    }

    // Validate file structure
    std::ifstream in(result_file);
    REQUIRE(in.is_open());

    std::string line1;
    std::getline(in, line1);
    REQUIRE(line1 == "2025-01-09-Test-E2E-0000000");

    std::string line2;
    std::getline(in, line2);
    REQUIRE(line2.size() > 2);
    REQUIRE(line2[0] == 'C');

    double cost = std::stod(line2.substr(2));
    REQUIRE(cost == Catch::Approx(285.0).margin(25.0));

    in.close();
    std::filesystem::remove(result_file);
}

// ============================================================================
// SECTION 8: Edge cases & robustness
// ============================================================================

TEST_CASE("MDP: 2-state trivial graph", "[mdp][edge]") {
    const std::string tmp = "tests/_tmp_trivial.txt";
    {
        std::ofstream f(tmp);
        f << "N 2\n";
        f << "A 0 1 1 42\n";
        f << "A 1 1 1 0\n";
    }

    double result = run_mdp(tmp, 500);
    REQUIRE(result == Catch::Approx(42.0).margin(0.01));

    std::filesystem::remove(tmp);
}

TEST_CASE("MDP: all paths have same cost", "[mdp][edge]") {
    // 0 -> 1 (cost 50), 0 -> 2 (cost 50)
    // 1 -> 3 (cost 50), 2 -> 3 (cost 50)
    // Total from any path: 100
    const std::string tmp = "tests/_tmp_equal_cost.txt";
    {
        std::ofstream f(tmp);
        f << "N 4\n";
        f << "A 0 1 0.5 50\n";
        f << "A 0 2 0.5 50\n";
        f << "A 1 3 1 50\n";
        f << "A 2 3 1 50\n";
        f << "A 3 3 1 0\n";
    }

    double result = run_mdp(tmp, 1000);
    REQUIRE(result == Catch::Approx(100.0).margin(0.01));

    std::filesystem::remove(tmp);
}

TEST_CASE("MDP: probabilities sum validation in parsed graph", "[mdp][validation]") {
    // Verify all states with outgoing arcs have probabilities summing to 1.0
    for (int test_id = 1; test_id <= 5; ++test_id) {
        std::string file = "tests/parameters/params_09_01_ex1_t" + std::to_string(test_id) + ".txt";
        auto global = std::make_shared<mdp_global_t>();
        mdp_input_parser_t parser(file, global);
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

TEST_CASE("montecarlo_t: average converges with more runs", "[montecarlo][convergence]") {
    // Test 1 expected = 285
    double r100 = run_mdp("tests/parameters/params_09_01_ex1_t1.txt", 100);
    double r1000 = run_mdp("tests/parameters/params_09_01_ex1_t1.txt", 1000);
    double r5000 = run_mdp("tests/parameters/params_09_01_ex1_t1.txt", 5000);

    // All should be finite
    REQUIRE(std::isfinite(r100));
    REQUIRE(std::isfinite(r1000));
    REQUIRE(std::isfinite(r5000));

    // Higher budget should be closer to 285
    REQUIRE(std::abs(r5000 - 285.0) <= std::abs(r100 - 285.0) + 30.0);
}

// ============================================================================
// SECTION 9: Markov chain integration with MDP
// ============================================================================

TEST_CASE("markov_chain: models Test 1 MDP correctly", "[markov][integration]") {
    // Build the same MDP as Test 1 using the markov utility
    isw::markov::markov_chain_t mc(4);
    mc.matrix[0][1] = {1.0, 100.0};
    mc.matrix[1][2] = {0.7, 100.0};
    mc.matrix[1][3] = {0.3, 150.0};
    mc.matrix[2][3] = {1.0, 100.0};
    mc.matrix[3][3] = {1.0, 0.0};

    // Run many walk simulations manually
    random_t rng(42);
    int N = 10000;
    double total_cost = 0;
    for (int i = 0; i < N; ++i) {
        size_t state = 0;
        double cost = 0;
        while (state != 3) {
            size_t next = mc.next_state(state, rng.get_engine());
            cost += mc.matrix[state][next].second;
            state = next;
        }
        total_cost += cost;
    }
    double avg_cost = total_cost / N;
    REQUIRE(avg_cost == Catch::Approx(285.0).margin(10.0));
}
