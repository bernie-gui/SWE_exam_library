/*
 * File: network.hpp
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
 *	This header file defines the network_t class and scanner_t thread for network operations in the simulation system.
 */
#pragma once
#include <vector>

#include "process.hpp"

namespace isw
{
    /**
     * @brief Represents a network process in the simulation system.
     */
    class network_t : public isw::process_t
    {
    };

    /**
     * @brief Thread responsible for scanning processes and dispatching messages between them.
     * @details This class implements a message passing mechanism where it randomly selects processes
     * to check for outgoing messages and forwards them to the appropriate input channels.
     */
    class scanner_t : public isw::thread_t
    {
    public:
        /**
         * @brief Constructs a scanner thread with specified timing parameters.
         * @param[in] c_time Creation time.
         * @param[in] s_time Start time.
         * @param[in] th_time Thread time, defaults to 0.0.
         */
        scanner_t( double c_time, double s_time, double th_time = 0.0 );
        /**
         * @brief Executes the scanning and message dispatching logic.
         * @details Randomly shuffles the process list when all have been scanned, then selects the next process,
         * checks its output channel, and if a message is present, forwards it to the receiver's input channel.
         * Rebuilds the scanner list if the number of processes has changed.
         */
        virtual void fun() override;
        virtual void on_start_scan();
        virtual bool filter(network::channel_t &current_channel);
        /**
         * @brief Initializes the scanner with the current list of processes.
         * @details Populates the _scanner vector with indices from 0 to processes.size()-1 and sets _current to 0.
         */
        virtual void init() override;

    protected:
        /** @brief List of process indices to scan. */
        std::vector< size_t > _scanner;
        /** @brief Current index in the scanner list. */
        size_t _current; // scanned_idx
    };
} // namespace isw
