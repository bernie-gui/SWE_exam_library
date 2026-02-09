#include <vector>
#include "utils/backtracking.hpp"
#include <functional>
#include <memory>
#include <iostream>
using namespace isw;

int main() {
    std::function<double(std::shared_ptr<std::vector<int>>)> f = [](auto) 
        {return 2.0;};
    auto res = arg_min_max({std::make_pair(-100, 100)}, f, utils::arg_strat::MIN);
    for (auto &el : *res) {
        std::cout << el[0] << std::endl;
    }
}