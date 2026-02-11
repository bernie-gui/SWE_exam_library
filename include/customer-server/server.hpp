/*
 * File: server.hpp
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
 *	This header file defines the server_t type for custormer-server simulations.
 */
#pragma once

#include <cstddef>
#include <functional>
#include <vector>
#include "process.hpp"

namespace isw::cs {
    using fill = std::function<size_t(size_t)>;

    class server_t : public process_t {
        public:

            server_t(size_t db_size, fill init, std::string name);

            void init() override;

            std::vector<size_t> database;
        
        private:
            fill _init;
    };
}

