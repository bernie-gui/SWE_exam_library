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
 *	This file implements the supplier_t and supplier_thread_t classes for custormer-server simulations.
 */
#include <memory>
#include "process.hpp"
#include "utils/customer-server/supplier.hpp"
#include "utils/customer-server/utils.hpp"

using namespace isw::cs;

supplier_t::supplier_t(std::string name) : process_t(name) {}

std::shared_ptr<supplier_t> supplier_t::create_process(double c_time, pick policy, ask item, ask quantity, 
    world_key_t servers, set compute, set sleep, double s_time, double th_time, std::string name) {
        auto res = std::make_shared<supplier_t>(name);
        res->add_thread(std::make_shared<supplier_thread_t>(c_time, policy, item, quantity, 
            servers, compute, sleep, s_time, th_time));
        return res;
    }

supplier_thread_t::supplier_thread_t(double c_time, pick policy, ask item, ask quantity, 
    world_key_t servers, set compute, set sleep, double s_time, double th_time) : thread_t(c_time, s_time, th_time), 
        _policy(policy), _item(item), _quantity(quantity), _servers(servers), _compute(compute), _sleep(sleep) {}

void supplier_thread_t::fun() {
    size_t choice = _policy();
    size_t item = _item(choice);
    size_t quant = _quantity(choice);
    cs::request_t send;
    send.item = item;
    send.quantity = quant;
    send_message(_servers, choice, send);
    if (_compute)
        set_compute_time(_compute());
    if (_sleep)
        set_sleep_time(_sleep());
}
        