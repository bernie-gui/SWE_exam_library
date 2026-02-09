/*
 * File: pid_network.hpp
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
 *	This header file defines the pid_scanner_t thread for network operations in the simulation system.
 */
#pragma once

#include <cstddef>
#include "network/network.hpp"

// TODO: documentation
namespace isw::network
{
    constexpr double S_TIME_MIN = 0;
    constexpr double S_TIME_MAX = 1800;
    constexpr double KP = 0.1;
    constexpr double KI = 0.05;
    constexpr double KD = 0.01;
    constexpr double DV_ALPHA = 0.2;
    constexpr double ERROR_THRESHOLD = 0.1;
    
    /**
     * @brief Thread responsible for scanning processes and dispatching messages between them.
     * @details This class implements a message passing mechanism where it randomly selects processes
     * to check for outgoing messages and forwards them to the appropriate input channels.
     */
    class pid_scanner_t : public scanner_t
    {
    public:
        /**
         * @brief Constructs a pid-scanner thread with specified timing parameters.
         * @param[in] th_time Thread time, defaults to 0.0.
         */
        pid_scanner_t( double obj_occupancy, double th_time = 0.0 );
        virtual void on_start_scan() override;
        /**
         * @brief Initializes the scanner with the current list of processes.
         * @details Populates the _scanner vector with indices from 0 to processes.size()-1 and sets _current to 0.
         */
        virtual void init() override;

    protected:
        double _obj_occupancy, _integral, 
            _prev_error, _prev_dv, _last_time;
    };
}
