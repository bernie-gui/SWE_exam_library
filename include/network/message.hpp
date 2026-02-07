/*
 * File: message.hpp
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
 *	This header file defines the message_t class and channel_t type for network message passing in the simulation.
 */
#pragma once
#include <memory>
#include <queue>
#include "common.hpp"

namespace isw::network
{
    /**
     * @brief Base class for messages in the network simulation.
     * @details Provides common attributes for message passing: sender ID, receiver ID, and timestamp.
     */
    class message_t
    {
    public:
        virtual ~message_t() = default; /**< @brief Virtual destructor for polymorphic deletion. */
        size_t receiver;                /**< @brief ID of the receiving process. */
        double timestamp;               /**< @brief Time when the message was sent. */
        size_t sender;                  /**< @brief ID of the sending process. */
        size_t sender_rel;				/**< @brief Relative ID of the sending process. */
        world_key_t world_key;        	/**< @brief World key of the sending process. */
    };

    /** @brief Type alias for a message channel, implemented as a queue of shared pointers to messages. */
    using channel_t = std::queue< std::shared_ptr< message_t > >;
} // namespace isw::network

// HOW TO USE DYNAMIC CAST WITH message_t
// using namespace isw::network;
// struct mio_messaggio : message_t {
// 	int ciao;
//  	bool sloop;
// };
// mio_messaggio(ciao, slop)
// void test() {
// 	std::shared_ptr<message_t> msg = std::make_shared<mio_messaggio>();
//
//
// 	// 1* way
// 	if (auto casted = std::dynamic_pointer_cast<mio_messaggio>(msg)) {
//   		casted->ciao = 42;
// 		casted->sloop = true;
// 	} else {
//   		// handle error
// 	}


// 	// 2* way C like (NOTE: this lose the smart pointer features)
// 	if (auto casted = dynamic_cast<mio_messaggio*>(msg.get())) {
// 	    casted->ciao = 42;
// 		casted->sloop = true;
// 	} else {
//   		// handle error
// 	}
// }
