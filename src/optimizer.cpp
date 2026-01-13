/*
 * File: optimizer.cpp
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
 *	This file implements the optimizer_t class methods for optimization using Monte Carlo methods.
 */
#include "optimizer.hpp"
#include <cstddef>
#include <limits>
#include <stdexcept>

using namespace isw;


optimizer_t::optimizer_t( std::shared_ptr< global_t > global ) : _global( global ) {}

/*
 * Run the optimization process
 */
void optimizer_t::optimize( optimizer_strategy strategy, std::vector< double > min_solution,
                            std::vector< double > max_solution )
{
    assert( min_solution.size() == max_solution.size() );
    size_t n_params = min_solution.size();
    std::vector< double > arguments( n_params );
    auto random = _global->get_random();
    // sol parametro (argmuents)
    // obj è lo scalare (risltato di obj_fun che lui scrive in global optimizer result)

    double best_obj_so_far = strategy == optimizer_strategy::MAXIMIZE ? std::numeric_limits< double >::lowest()
                                                                      : std::numeric_limits< double >::max();
    std::vector< double > best_sol_so_far( n_params );

    for ( size_t i = 0; i < _global->optimizer_budget(); i++ )
    {
        for ( size_t i = 0; i < n_params; i++ )
        {
            arguments[i] = random->uniform_range( min_solution[i], max_solution[i] );
        }
        double obj_resul = obj_fun( arguments );
        switch ( strategy )
        {
            case optimizer_strategy::MINIMIZE:
                if ( obj_resul < best_obj_so_far )
                {
                    best_sol_so_far = arguments;
                    best_obj_so_far = obj_resul;
                };
                break;
            case optimizer_strategy::MAXIMIZE:
                if ( obj_resul > best_obj_so_far )
                {
                    best_sol_so_far = arguments;
                    best_obj_so_far = obj_resul;
                };
                break;
            default:
                throw std::runtime_error( "not implemented" );
        }
    }
    // Store best result found so far
    _global->set_optimizer_result( best_obj_so_far );
    _global->set_optimizer_optimal_parameters( best_sol_so_far );
}

void optimizer_t::optimize( optimizer_strategy strategy, double min_solution, double max_solution )
{
    optimize( strategy, std::vector< double >( 1, min_solution ), std::vector< double >( 1, max_solution ) );
}
