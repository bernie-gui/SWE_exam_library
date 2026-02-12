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

//TODO: documentation
namespace isw::cs {
    class supplier_thread_t;
    using pick = std::function<size_t(void)>;
    using ask = std::function<size_t(size_t)>;
    using set = std::function<double(void)>;

    class supplier_t : public process_t {
        public:
            supplier_t(std::string name = "default server");

            static std::shared_ptr<supplier_t> create_process(double c_time, pick policy, ask item, ask quantity, 
                world_key_t servers, set compute = 0, set sleep = 0, 
                double s_time = 0, double th_time = 0, 
                std::string name = "default server");
    };

    class supplier_thread_t : public thread_t {
        public:
            supplier_thread_t(double c_time, pick policy, ask item, ask quantity, 
                world_key_t servers, set compute = 0, set sleep = 0, 
                double s_time = 0, double th_time = 0);

            void fun() override;

        private:
            pick _policy;
            ask _item, _quantity;
            world_key_t _servers;
            set _compute, _sleep;
    };
}