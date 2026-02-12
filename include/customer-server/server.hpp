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

//TODO: documentation
namespace isw::cs {
    class server_t;
    template<class mes_type>
    class server_thread_t;
    using fill = std::function<size_t(size_t)>;
    template <class mes_type>
    using sorter = std::function<void(std::shared_ptr<server_thread_t<mes_type>>, std::shared_ptr<mes_type>)>;
    template <class mes_type>
    using binding = std::unordered_map< world_key_t, sorter<mes_type> >;

    class server_t : public process_t {
        public:

            server_t(size_t db_size, fill init, std::string name = "default server");

            void init() override;

            template <class mes_type>
            static std::shared_ptr<server_t> create_process(size_t db_size, fill init, 
                double c_time, binding<mes_type> &bindings, double s_time = 0, double th_time = 0, std::string name = "default server") {
                    auto res = std::make_shared<server_t>(db_size, init, name);
                    res->add_thread(std::make_shared<server_thread_t<mes_type>>(c_time, bindings, s_time, th_time));
                    return res;
                }
            
            template <class mes_type>
            static std::shared_ptr<server_t> create_process(size_t db_size, fill init, 
                double c_time, binding<mes_type> &&bindings, double s_time = 0, double th_time = 0, std::string name = "default server") {
                    auto res = std::make_shared<server_t>(db_size, init, name);
                    res->add_thread(std::make_shared<server_thread_t<mes_type>>(c_time, std::move(bindings), s_time, th_time));
                    return res;
                }

            std::vector<size_t> database;
        
        private:
            fill _init;
    };

    template<class mes_type>
    class server_thread_t : public thread_t, public std::enable_shared_from_this<server_thread_t<mes_type>> {
        public:

            server_thread_t(double c_time, binding<mes_type> &bindings, double s_time = 0,
                double th_time = 0) : thread_t(c_time, s_time, th_time), _bindings(bindings) {}
            
            server_thread_t(double c_time, binding<mes_type> &&bindings, double s_time = 0,
                double th_time = 0) : thread_t(c_time, s_time, th_time), _bindings(std::move(bindings)) {}

            void fun() override {
                std::shared_ptr< mes_type > msg;
                if ((msg = receive_message< mes_type >()) == nullptr) return;
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
            }

        private:
            binding<mes_type> _bindings;
    };
}

