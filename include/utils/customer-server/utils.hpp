/*
 * File: request.hpp
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
 *	This header file defines the request_t class for customer-server message passing in the simulation.
 */
#pragma once
#include <cstddef>
#include "network/message.hpp"

namespace isw::cs {
    /** @brief Function type returning a double, used for dynamic compute and sleep time setters. */
    using set = std::function<double(void)>;

    /**
     * @brief Message type for customer-server request passing.
     * @details Extends network::message_t with fields to identify the requested item, tag, and quantity.
     */
    class request_t : public network::message_t {
        public:
            /** @brief Index of the requested item in the server database. */
            size_t item;
            /** @brief Tag identifying the type of request (e.g. buy, restock). */
            size_t tag;
            /** @brief Quantity to buy (negative) or restock (positive). */
            int  quantity;
    };
}
