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
#pragma once

#include <utility>
#include <vector>
#include <random>

namespace isw::markov {
    /**
     * @brief Represents a discrete-time Markov chain with transition probabilities and costs.
     * @details Each entry matrix[i][j] is a pair {probability, cost} for the transition from state i to state j.
     *   The probabilities in each row must sum to 1.
     */
    class markov_chain_t {
        public:
            /**
             * @brief Transition matrix storing {probability, cost} pairs.
             * @details matrix[i][j].first is the transition probability from state i to j,
             *   matrix[i][j].second is the cost associated with that transition.
             */
            std::vector<std::vector<std::pair<double, double>>> matrix;

            /**
             * @brief Samples the next state from the current state using the transition probabilities.
             * @param[in] current The current state index.
             * @param[in,out] engine Mersenne Twister random engine used for sampling.
             * @return The next state index.
             * @throws std::runtime_error If the Markov chain transition probabilities are not properly defined.
             */
            size_t next_state(size_t current, std::mt19937_64 &engine);

            /**
             * @brief Constructs a Markov chain with a given number of states.
             * @param[in] size The number of states. Allocates an size x size matrix.
             */
            markov_chain_t(size_t size);

            /**
             * @brief Default constructor.
             * @details Creates an empty Markov chain with no states.
             */
            markov_chain_t();
    };
}
