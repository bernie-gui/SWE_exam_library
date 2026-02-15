/*
 * File: markov.hpp
 * Copyright (c) 2025 bernie_gui, uniquadev, SepeFr.
 *
 * This file is part of SWE_exam_library
 *
 * SWE_exam_library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SWE_exam_library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * University: Sapienza University of Rome
 * Instructor: Enrico Tronci
 * Academic Year: 2025-2026
 *
 * Description:
 *	This header file defines markov-related utilities.
 */
#include "utils/markov/markov.hpp"
#include <random>
#include <stdexcept>

using namespace isw::markov;

size_t markov_chain::next_state(size_t state, std::mt19937_64 &engine) {
    size_t size = matrix.size();
    std::uniform_real_distribution<double> unif(0, 1);
    double prob = unif(engine);
    double accum = 0;
    for (size_t i = 0; i < size; i++) {
        accum += matrix[state][i].first;
        if (prob <= accum) return i;
    }
    throw std::runtime_error(" markov_chain not defined properly ");
}

markov_chain::markov_chain(size_t size) : matrix(size) {
    for (auto &el : matrix) el.resize(size);
}

markov_chain::markov_chain() : matrix() {}