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
 *	This header file defines the server_t and server_thread_t classes for custormer-server simulations.
 */
#pragma once

#include <string>
#include <cstddef>
#include <functional>
#include <memory>
#include <utility>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include "common.hpp"
#include "process.hpp"
#include "utils/customer-server/utils.hpp"

namespace isw::cs {
    class server_t;
    template<class mes_type>
    class server_thread_t;
    /** @brief Function type mapping a database index to an initial value, used for server database initialization. */
    using fill = std::function<size_t(size_t)>;
    /**
     * @brief Function type that handles an incoming message on a server thread.
     * @tparam mes_type The message type (must extend network::message_t).
     */
    template <class mes_type>
    using sorter = std::function<void(std::shared_ptr<server_thread_t<mes_type>>, std::shared_ptr<mes_type>)>;
    /**
     * @brief Map from world keys to message handler functions.
     * @tparam mes_type The message type (must extend network::message_t).
     * @details Routes incoming messages to the appropriate handler based on the sender's world key.
     */
    template <class mes_type>
    using binding = std::unordered_map< world_key_t, sorter<mes_type> >;

    /**
     * @brief Server process for customer-server simulations.
     * @details Manages a database of item quantities and routes incoming messages to handlers via bindings.
     *   Extends process_t with a database vector and a factory method for creating fully configured server processes.
     */
    class server_t : public process_t {
        public:

            /**
             * @brief Constructs a server with a database of given size.
             * @param[in] db_size Number of items in the database.
             * @param[in] init Initialization function mapping each index to its starting value.
             * @param[in] name Process name, defaults to "default server".
             */
            server_t(size_t db_size, fill init, std::string name = "default server");

            /**
             * @brief Initializes the server and populates the database using the init function.
             */
            void init() override;

            /**
             * @brief Factory method creating a fully configured server process with its thread (lvalue bindings).
             * @tparam mes_type The message type.
             * @param[in] db_size Number of items in the database.
             * @param[in] init Initialization function for the database.
             * @param[in] c_time Thread compute time.
             * @param[in] bindings Message handler bindings (lvalue reference).
             * @param[in] compute Optional dynamic compute time setter.
             * @param[in] sleep Optional dynamic sleep time setter.
             * @param[in] s_time Thread sleep time, defaults to 0.
             * @param[in] th_time Thread threshold time, defaults to 0.
             * @param[in] name Process name, defaults to "default server".
             * @return Shared pointer to the created server process.
             */
            template <class mes_type>
            static std::shared_ptr<server_t> create_process(size_t db_size, fill init, 
                double c_time, binding<mes_type> &bindings, set compute = 0, set sleep = 0, 
                double s_time = 0, double th_time = 0, std::string name = "default server") {
                    auto res = std::make_shared<server_t>(db_size, init, name);
                    res->add_thread(std::make_shared<server_thread_t<mes_type>>(c_time, bindings, compute, 
                                sleep, s_time, th_time));
                    return res;
                }
            
            /**
             * @brief Factory method creating a fully configured server process with its thread (rvalue bindings).
             * @details Moves the bindings into the server thread.
             */
            template <class mes_type>
            static std::shared_ptr<server_t> create_process(size_t db_size, fill init, 
                double c_time, binding<mes_type> &&bindings, set compute = 0, set sleep = 0,
                double s_time = 0, double th_time = 0, std::string name = "default server") {
                    auto res = std::make_shared<server_t>(db_size, init, name);
                    res->add_thread(std::make_shared<server_thread_t<mes_type>>(c_time, std::move(bindings), 
                        compute, sleep, s_time, th_time));
                    return res;
                }

            /** @brief Database of item quantities managed by this server. */
            std::vector<size_t> database;
        
        private:
            /** @brief Initialization function for populating the database. */
            fill _init;
    };

    /**
     * @brief Thread implementation for server message processing.
     * @tparam mes_type The message type (must extend network::message_t).
     * @details Receives messages, routes them through bindings based on sender world key,
     *   and optionally updates compute and sleep times.
     */
    template<class mes_type>
    class server_thread_t : public thread_t, public std::enable_shared_from_this<server_thread_t<mes_type>> {
        public:

            /**
             * @brief Constructs a server thread with message handler bindings (lvalue).
             * @param[in] c_time Compute time.
             * @param[in] bindings Message handler bindings.
             * @param[in] compute Optional dynamic compute time setter.
             * @param[in] sleep Optional dynamic sleep time setter.
             * @param[in] s_time Sleep time, defaults to 0.
             * @param[in] th_time Threshold time, defaults to 0.
             */
            server_thread_t(double c_time, binding<mes_type> &bindings, set compute = 0, set sleep = 0,
                double s_time = 0, double th_time = 0) : thread_t(c_time, s_time, th_time), _bindings(bindings), 
                _compute(compute), _sleep(sleep) {}
            
            /**
             * @brief Constructs a server thread with message handler bindings (rvalue).
             * @details Moves the bindings into the thread.
             */
            server_thread_t(double c_time, binding<mes_type> &&bindings, set compute = 0, set sleep = 0,
                double s_time = 0, double th_time = 0) : thread_t(c_time, s_time, th_time), _bindings(std::move(bindings)),
                _compute(compute), _sleep(sleep) {}

            /**
             * @brief Processes incoming messages and routes them through bindings.
             * @details Receives a message, looks up the handler by world key, and invokes it.
             *   Throws std::runtime_error if no binding exists for the sender's world key.
             *   Optionally updates compute and sleep times via dynamic setters.
             */
            void fun() override {
                std::shared_ptr< mes_type > msg;
                if ((msg = receive_message< mes_type >()) != nullptr) {
                    auto it = _bindings.find(msg->world_key);
                    if (it != _bindings.end()) {
                        auto& sorter = it->second;
                        sorter(this->shared_from_this(), msg);
                    }
                    else {
                        std::string err(" unknown sender world: ");
                        err += msg->world_key;
                        throw std::runtime_error(err);
                    }
                };
                if (_compute)
                    set_compute_time(_compute());
                if (_sleep)
                    set_sleep_time(_sleep());
            }

        private:
            /** @brief Message handler bindings mapping world keys to handler functions. */
            binding<mes_type> _bindings;
            /** @brief Dynamic compute time setter. */
            /** @brief Dynamic sleep time setter. */
            set _compute, _sleep;
    };
}

