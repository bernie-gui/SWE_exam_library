/*
 * File: global.hpp
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
 *	This header file defines the global_t class for managing global state and configuration in the simulation system.
 */
#pragma once

#include <cstddef>
#include "network/message.hpp"
#include "random.hpp"

namespace isw
{
    /**
     * @brief Manages global state and configuration for the simulation system.
     * @details Holds random number generator, message channels, and simulation parameters like budgets and results.
     */
    class global_t
    {
    public:
        /**
         * @brief Default constructor.
         * @details Initializes the random number generator.
         */
        global_t();
        virtual ~global_t() = default;
        global_t( const global_t &other ) noexcept = default;
        /**
         * @brief Initializes simulation-specific state.
         * @details Clears all message channels and resets montecarlo current to 0.
         */
        virtual void init();
        /**
         * @brief Returns the shared pointer to the random number generator.
         * @return Shared pointer to random_t.
         */
        std::shared_ptr< random_t > get_random();
        /**
         * @brief Returns reference to the input message channels.
         * @return Reference to vector of input channels.
         * @note The returned reference can be modified in-place.
         */
        std::vector< network::channel_t > &get_channel_in();
        /**
         * @brief Returns reference to the output message channels.
         * @return Reference to vector of output channels.
         * @note The returned reference can be modified in-place.
         */
        std::vector< network::channel_t > &get_channel_out();
        /*
         * simulation parameters getters/setters
         */
        /**
         * @brief Gets the simulation horizon.
         * @return The horizon value.
         */
        double get_horizon() const;
        /**
         * @brief Sets the simulation horizon.
         * @param[in] horizon The horizon value to set.
         */
        void set_horizon( double horizon );

        /**
         * @brief Gets the Monte Carlo budget.
         * @return The Monte Carlo budget value.
         */
        size_t montecarlo_budget() const;
        /**
         * @brief Sets the Monte Carlo budget.
         * @param[in] montecarlo_budget The budget value to set.
         */
        void set_montecarlo_budget( size_t montecarlo_budget );

        /**
         * @brief Gets the optimizer budget.
         * @return The optimizer budget value.
         */
        size_t optimizer_budget() const;
        /**
         * @brief Sets the optimizer budget.
         * @param[in] optimizer_budget The budget value to set.
         */
        void set_optimizer_budget( size_t optimizer_budget );

        /**
         * @brief Gets the network number.
         * @return The network number value.
         */
        size_t network_number() const;
        /**
         * @brief Sets the network number.
         * @param[in] network_number The network number to set.
         */
        void set_network_number( size_t network_number );

        /**
         * @brief Gets the Monte Carlo average.
         * @return The Monte Carlo average value.
         */
        double get_montecarlo_avg() const;
        /**
         * @brief Sets the Monte Carlo average.
         * @param[in] avg The average value to set.
         */
        void set_montecarlo_avg( double avg );

        /**
         * @brief Gets the current Monte Carlo value.
         * @return The current Monte Carlo value.
         */
        double montecarlo_current() const;
        /**
         * @brief Sets the current Monte Carlo value.
         * @param[in] current The current value to set.
         */
        void set_montecarlo_current( double current );

        /**
         * @brief Gets the optimizer result.
         * @return The optimizer result value.
         */
        double get_optimizer_result() const;
        /**
         * @brief Sets the optimizer result.
         * @param[in] current The result value to set.
         */
        void set_optimizer_result( double current );

        std::vector< double > get_optimizer_optimal_parameters() const;
        void set_optimizer_optimal_parameters( std::vector< double > current );

        // WARNING: TO CREATE MONITOR METHODS MAKE SURE TO IMPLEMENT THEM VIA EXTENDING THIS CLASS
        /*
               ⢀⡞⠹⣦⠰⡞⠙⣆
               ⢴  ⣿⠐⡇ ⢻
               ⢸  ⣿⢸⡇ ⢸
              ⢀⡸⣄ ⣿⣨⡇ ⣟⡀
           ⢠⡶⠚⠉⢁⡀     ⡈⠉⠙⠲⣤⡀
         ⢀⡶⠋ ⢀⠔⠉      ⠈⠑⢄ ⠈⠻⡄
         ⣾⠁  ⠈ ⣠⣂⡄   ⣔⣢ ⠈   ⢹
         ⡇  ⢠⣠⣠⡌⠓⠁ ⡀ ⠙⠊⡄⢀⣀  ⢸
         ⢷⡀ ⠈⠁⠁  ⠈⠓⡓⠂  ⠉⠈⠁  ⡼
         ⠈⠳⣄⡀             ⣠⡞⠁
           ⢾ ⡄           ⣴⢸
           ⠈⢻⡄           ⡾⠊⠁
            ⠘⣇⢀⡀        ⡀⣷
             ⢿⣼⠉⠉⠙⠛⠛⠛⠛⠉⢹⣁⠟

         */

    private:
        /** @brief Random number generator. */
        std::shared_ptr< random_t > _rand;
        /** @brief Input message channels for each process. */
        std::vector< network::channel_t > _channel_in;
        /** @brief Output message channels for each process. */
        std::vector< network::channel_t > _channel_out;
        /** @brief Simulation horizon. */
        double _horizon;
        /** @brief Monte Carlo budget. */
        size_t _montecarlo_budget;
        /** @brief Optimizer budget. */
        size_t _optimizer_budget;
        /** @brief Optimizer alphas */
        std::vector< double > _optimizer_optimal_parameters;
        /** @brief Network number. */
        size_t _network_number;
        /** @brief Monte Carlo average. */
        double _montecarlo_avg;
        /** @brief Current Monte Carlo value. */
        double _montecarlo_current;
        /** @brief Optimizer result. */
        double _optimizer_result;
        /** @brief Arguments vector. */
        std::vector< double > arguments;
    }; //
} // namespace isw
