/*
 * File: supplier.hpp
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
 *	This header file defines the supplier_t and supplier_thread_t classes for custormer-server simulations.
 */
#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include "common.hpp"
#include "process.hpp"
#include "utils/customer-server/utils.hpp"

namespace isw::cs {
    class supplier_thread_t;
    /** @brief Function type returning a server index, used as the supplier's server selection policy. */
    using pick = std::function<size_t(void)>;
    /** @brief Function type mapping a server index to an item index or quantity. */
    using ask = std::function<size_t(size_t)>;

    /**
     * @brief Supplier process for customer-server simulations.
     * @details Periodically sends restock requests to servers based on a configurable policy.
     *   Extends process_t with a factory method for creating fully configured supplier processes.
     */
    class supplier_t : public process_t {
        public:
            /**
             * @brief Constructs a supplier process.
             * @param[in] name Process name, defaults to "default server".
             */
            supplier_t(std::string name = "default server");

            /**
             * @brief Factory method creating a fully configured supplier process with its thread.
             * @param[in] c_time Thread compute time.
             * @param[in] policy Function selecting which server to restock.
             * @param[in] item Function mapping a server index to the item to restock.
             * @param[in] quantity Function mapping a server index to the restock quantity.
             * @param[in] servers World key of the target server group.
             * @param[in] compute Optional dynamic compute time setter.
             * @param[in] sleep Optional dynamic sleep time setter.
             * @param[in] s_time Thread sleep time, defaults to 0.
             * @param[in] th_time Thread threshold time, defaults to 0.
             * @param[in] name Process name, defaults to "default server".
             * @return Shared pointer to the created supplier process.
             */
            static std::shared_ptr<supplier_t> create_process(double c_time, pick policy, 
                ask item, ask quantity, world_key_t servers, set compute = 0, set sleep = 0, 
                double s_time = 0, double th_time = 0, std::string name = "default server");
    };

    /**
     * @brief Thread implementation for supplier request generation.
     * @details Each invocation of fun() selects a server via the policy, determines the item and quantity,
     *   and sends a restock request message to the target server.
     */
    class supplier_thread_t : public thread_t {
        public:
            /**
             * @brief Constructs a supplier thread.
             * @param[in] c_time Compute time.
             * @param[in] policy Function selecting which server to restock.
             * @param[in] item Function mapping a server index to the item to restock.
             * @param[in] quantity Function mapping a server index to the restock quantity.
             * @param[in] servers World key of the target server group.
             * @param[in] compute Optional dynamic compute time setter.
             * @param[in] sleep Optional dynamic sleep time setter.
             * @param[in] s_time Sleep time, defaults to 0.
             * @param[in] th_time Threshold time, defaults to 0.
             */
            supplier_thread_t(double c_time, pick policy, ask item, ask quantity, 
                world_key_t servers, set compute = 0, set sleep = 0, 
                double s_time = 0, double th_time = 0);

            /**
             * @brief Generates and sends a restock request to a server.
             * @details Selects a server, determines item and quantity, sends a request_t message,
             *   and optionally updates compute and sleep times.
             */
            void fun() override;

        private:
            /** @brief Server selection policy. */
            pick _policy;
            /** @brief Item selection function. */
            /** @brief Quantity selection function. */
            ask _item, _quantity;
            /** @brief World key of the target server group. */
            world_key_t _servers;
            /** @brief Dynamic compute time setter. */
            /** @brief Dynamic sleep time setter. */
            set _compute, _sleep;
    };
}