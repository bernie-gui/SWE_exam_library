/*
 * File: optimizer.hpp
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
 *	This header file defines the optimizer_t class and optimizer_strategy enum for optimization algorithms using Monte
 * Carlo methods.
 */
#pragma once
#include <cassert>
#include <memory>
#include <vector>
#include "montecarlo.hpp"

namespace isw
{
    /** @brief Enumeration for optimization strategies. */
    enum class optimizer_strategy
    {
        MINIMIZE, /**< @brief Minimize the objective function. */
        MAXIMIZE  /**< @brief Maximize the objective function. */
    };

    /**
     * @brief Abstract base class for optimization algorithms using Monte Carlo methods.
     * @details Provides framework for running optimization by sampling parameters and evaluating objective functions.
     */
    class optimizer_t
    {
    public:
        /**
         * @brief Constructor.
         * @param[in] montecarlo Shared pointer to the Monte Carlo simulator.
         */
        optimizer_t( std::shared_ptr< global_t > global );

        /*
         * This must run the simulation and make the calculations
         */
        /**
         * @brief Pure virtual objective function to be implemented by subclasses.
         * @param[in,out] arguments The parameter vector for evaluation.
         * @details This method must run the simulation and compute the objective value, storing it in global
         * optimizer_result.
         */
        virtual double obj_fun( std::vector< double > &arguments ) = 0;

        /**
         * @brief Gets the global state, cast to type T.
         * @tparam T Type to cast to, defaults to global_t.
         * @return Shared pointer to the casted global.
         */
        template< typename T = global_t >
        std::shared_ptr< T > get_global()
        {
            auto casted = std::dynamic_pointer_cast< T >( _global );
            assert( casted.get() != nullptr );
            return casted;
        }

        /**
         * @brief Runs the optimization process for multi-dimensional parameters.
         * @param[in] strategy The optimization strategy (MINIMIZE or MAXIMIZE).
         * @param[in] min_solution Vector of minimum bounds for each parameter.
         * @param[in] max_solution Vector of maximum bounds for each parameter.
         * @throws std::runtime_error If strategy is not implemented.
         * @details Samples random parameters within bounds, evaluates objective function, and tracks the best solution.
         */
        void optimize( optimizer_strategy strategy, std::vector< double > min_solution,
                       std::vector< double > max_solution );

        /**
         * @brief Runs the optimization process for single-dimensional parameter.
         * @param[in] strategy The optimization strategy.
         * @param[in] min_solution Minimum bound.
         * @param[in] max_solution Maximum bound.
         * @details Delegates to the vector version with single-element vectors.
         */
        void optimize( optimizer_strategy strategy, double min_solution, double max_solution );

    private:
        /** @brief The Monte Carlo simulator instance. */
        std::shared_ptr< global_t > _global;
    };

} // namespace isw
