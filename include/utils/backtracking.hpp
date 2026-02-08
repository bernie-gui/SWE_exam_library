#pragma once

#include ".base/hashing.hpp"
#include <cstddef>
#include <functional>
#include <limits>
#include <memory>
#include <random>
#include <unordered_set>
#include <utility>
#include <vector>

// TODO: documentation
namespace isw {
    namespace backtrack {
        enum class arg_strat {
        MIN, MAX
        };

        template<typename param_t>
        using bucket = std::shared_ptr<std::unordered_set<std::vector<param_t>>>;

        template<typename param_t, typename res_t>
        using couple = std::pair<bucket<param_t>, res_t>;

        template<typename param_t, typename res_t>
        couple<param_t, res_t> backtrack(std::vector<std::pair<param_t, param_t>> &ranges, std::shared_ptr<std::vector<param_t>> ancilla, arg_strat strategy, 
                                                            couple<param_t, res_t> info, std::function<res_t(std::shared_ptr<std::vector<param_t>>)> f, std::size_t i = 0) {
            if (i == ancilla->size()) {
                res_t res = f(ancilla);
                if (strategy == arg_strat::MAX ? res > info.second : res < info.second) {
                    info.first->clear();
                    info.first->insert(*ancilla);
                    info.second = res;
                }
                else if (res == info.second) {
                    info.first->insert(*ancilla);
                }
                return info;
            }
            param_t val;
            couple<param_t, res_t> temp = info;
            for (val = ranges[i].first; val <= ranges[i].second; val++) {
                (*ancilla)[i] = val;
                temp = backtrack(ranges, ancilla, strategy, temp, f, i + 1);
            }
            return temp;
        }

        // to use only for discrete types
        template<typename param_t, typename res_t>
        bucket<param_t> arg_min_max(std::vector<std::pair<param_t, param_t>> &ranges, 
                                    std::function<res_t(std::shared_ptr<std::vector<param_t>>)> f, arg_strat strategy) {
            couple<param_t, res_t> info = {std::make_shared<std::unordered_set<std::vector<param_t>>>(), 
                        (strategy == arg_strat::MAX) ? std::numeric_limits<res_t>::lowest() : std::numeric_limits<res_t>::max()};
            auto ancilla = std::make_shared<std::vector<param_t>>(ranges.size());
            backtrack(ranges, ancilla, strategy, info, f);
            return info.first;
        }

        template<typename param_t, typename res_t>
        bucket<param_t> arg_min_max(std::vector<std::pair<param_t, param_t>> &&ranges, 
                                                            std::function<res_t(std::shared_ptr<std::vector<param_t>>)> f, arg_strat strategy) {
            auto rang = std::move(ranges);
            return arg_min_max(rang, f, strategy);
        }

        template<typename param_t, typename res_t>
        couple<param_t, res_t> free_backtrack(std::vector<std::unordered_set<param_t>> &ranges, std::shared_ptr<std::vector<param_t>> ancilla, arg_strat strategy, 
                                                            couple<param_t, res_t> info, std::function<res_t(std::shared_ptr<std::vector<param_t>>)> f, std::size_t i = 0) {
            if (i == ancilla->size()) {
                res_t res = f(ancilla);
                if (strategy == arg_strat::MAX ? res > info.second : res < info.second) {
                    info.first->clear();
                    info.first->insert(*ancilla);
                    info.second = res;
                }
                else if (res == info.second) {
                    info.first->insert(*ancilla);
                }
                return info;
            }
            couple<param_t, res_t> temp = info;
            for (const auto &el : ranges[i]) {
                (*ancilla)[i] = el;
                temp = free_backtrack(ranges, ancilla, strategy, temp, f, i + 1);
            }
            return temp;
        }

        template<typename param_t, typename res_t>
        bucket<param_t> arg_min_max(std::vector<std::unordered_set<param_t>> &ranges, 
                                                            std::function<res_t(std::shared_ptr<std::vector<param_t>>)> f, arg_strat strategy) {
            couple<param_t, res_t> info = {std::make_shared<std::unordered_set<std::vector<param_t>>>(), 
                        (strategy == arg_strat::MAX) ? std::numeric_limits<res_t>::lowest() : std::numeric_limits<res_t>::max()};
            auto ancilla = std::make_shared<std::vector<param_t>>(ranges.size());
            free_backtrack(ranges, ancilla, strategy, info, f);
            return info.first;
        }

        template<typename param_t, typename res_t>
        bucket<param_t> arg_min_max(std::vector<std::unordered_set<param_t>> &&ranges, 
                                                            std::function<res_t(std::shared_ptr<std::vector<param_t>>)> f, arg_strat strategy) {
            auto rang = std::move(ranges);
            return arg_min_max(rang, f, strategy);
        }

        template<typename param_t> 
        std::shared_ptr<std::vector<param_t>> get_unif_random(bucket<param_t> bucket, std::mt19937_64 &engine) {
            std::uniform_int_distribution< int > dist(0, bucket->size()-1);
            int idx = dist(engine);
            auto it = bucket->begin();
            std::advance( it, idx );
            return std::make_shared<std::vector<param_t>>(*it);
        }; 
    }
}