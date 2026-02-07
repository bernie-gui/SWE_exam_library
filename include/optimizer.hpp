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
#include <algorithm>
#include <cassert>
#include <memory>
#include <stdexcept>
#include <vector>
#include "global.hpp"

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
    template <typename param_t = double, typename res_t = double>
    class optimizer_t
    {
    public:
        /**
         * @brief Constructor.
         * @param[in] montecarlo Shared pointer to the Monte Carlo simulator.
         */
        optimizer_t( std::shared_ptr< global_t > global ) : _global( global ) {};

        /*
         * This must run the simulation and make the calculations
         */
        /**
         * @brief Pure virtual objective function to be implemented by subclasses.
         * @param[in,out] arguments The parameter vector for evaluation.
         * @details This method must run the simulation and compute the objective value, storing it in global
         * optimizer_result.
         */
        virtual res_t obj_fun( std::vector< param_t > &arguments ) = 0;

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
        void optimize( optimizer_strategy strategy, std::vector< param_t > min_solution, std::vector< param_t > max_solution ) {
            assert( min_solution.size() == max_solution.size() );
            size_t n_params = min_solution.size();
            std::vector< param_t > arguments( n_params );
            auto random = _global->get_random();
            // sol parametro (argmuents)
            // obj è lo scalare (risltato di obj_fun che lui scrive in global optimizer result)

            res_t best_res_so_far = strategy == optimizer_strategy::MAXIMIZE ? std::numeric_limits< res_t >::lowest()
                                                                            : std::numeric_limits< res_t >::max();
            std::vector< param_t > best_param_so_far( n_params );

            for ( size_t i = 0; i < _global->optimizer_budget(); i++ )
            {
                for ( size_t i = 0; i < n_params; i++ )
                {
                    arguments[i] = random->uniform_range( min_solution[i], max_solution[i] );
                }
                res_t obj_resul = obj_fun( arguments );
                switch ( strategy )
                {
                    case optimizer_strategy::MINIMIZE:
                        if ( obj_resul < best_res_so_far )
                        {
                            best_param_so_far = arguments;
                            best_res_so_far = obj_resul;
                        };
                        break;
                    case optimizer_strategy::MAXIMIZE:
                        if ( obj_resul > best_res_so_far )
                        {
                            best_param_so_far = arguments;
                            best_res_so_far = obj_resul;
                        };
                        break;
                    default:
                        throw std::runtime_error( "not implemented" );
                }
            }
            // Store best result found so far
            std::vector<double> temp(best_param_so_far.size());
            std::copy(best_param_so_far.begin(), best_param_so_far.end(), temp.begin());
            _global->set_optimizer_result( static_cast<double>(best_res_so_far) );
            _global->set_optimizer_optimal_parameters( temp );
        }

        /**
         * @brief Runs the optimization process for single-dimensional parameter.
         * @param[in] strategy The optimization strategy.
         * @param[in] min_solution Minimum bound.
         * @param[in] max_solution Maximum bound.
         * @details Delegates to the vector version with single-element vectors.
         */
        void optimize( optimizer_strategy strategy, param_t min_solution, param_t max_solution ) {
            optimize( strategy, std::vector< param_t >( 1, min_solution ), std::vector< param_t >( 1, max_solution ) );
        }

    private:
        /** @brief The Monte Carlo simulator instance. */
        std::shared_ptr< global_t > _global;
    };

} // namespace isw
