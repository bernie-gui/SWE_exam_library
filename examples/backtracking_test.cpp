#include <vector>
#include "utils/backtracking.hpp"
#include <functional>
#include <memory>
#include <iostream>
using namespace isw;

int main() {
    std::function<int(std::shared_ptr<std::vector<int>>)> f = [](auto x) {return (*x)[0]*(*x)[0];};
    auto res = arg_min_max({{-10, 10}}, f, arg_strat::MIN);
    for (auto &el : *res) {
        std::cout << el[0] << std::endl;
    }
}