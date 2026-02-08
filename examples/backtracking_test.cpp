#include <cmath>
#include <utility>
#include <vector>
#include "utils/backtracking.hpp"
#include <functional>
#include <memory>
#include <iostream>
using namespace isw;

int main() {
    std::function<double(std::shared_ptr<std::vector<int>>)> f = [](auto x) 
        {return (*x)[0] * (*x)[1];};
    auto res = arg_min_max({{-100, 3, 100}, {-100, 100}}, f, arg_strat::MAX);
    for (auto &el : *res) {
        std::cout << el[0] << " | " << el[1] << std::endl;
    }
}