#pragma once
#include <functional>
#include <unordered_set>
#include <utility>
#include <vector>

enum class arg_strat {
    MIN, MAX
};

template<typename param_t, typename res_t>
std::unordered_set<std::vector<param_t>> argmin_max(std::vector<std::pair<param_t, param_t>> ranges, 
                                                    std::function<res_t(std::vector<param_t>)> f);

template<typename param_t, typename res_t>
void f();