/*
 * File: main.cpp
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
 */
#include <cassert>
#include <memory>
#include <stdexcept>

#include "common.hpp"
#include "global.hpp"
#include "process.hpp"
#include "system.hpp"
#include "simulator.hpp"
#include "random.hpp"

#include "montecarlo.hpp"
#include "optimizer.hpp"

#include "io/input_parser.hpp"
#include "io/logger.hpp"
#include "io/output_writer.hpp"

#include "network/message.hpp"
#include "network/network.hpp"

using namespace isw;


class my_global_t : public global_t
{
public:
	my_global_t() : global_t() {}
};

class my_input_parser_t : public input_parser_t
{
public:
    my_input_parser_t( std::shared_ptr< global_t > global ) :
        input_parser_t( "parameters.txt" )
    {
    }

    size_t n_states;
    void parse() override
    {
    	throw std::runtime_error("Not implemented");
    }
};

class my_thread_t : public thread_t
{
public:
	my_thread_t() : thread_t( .1, .1, .1 ) {}

	void fun() override {
		throw std::runtime_error("Not implemented");
	}
};

class my_simulator_t : public simulator_t
{
public:
    using simulator_t::simulator_t;

    virtual bool should_terminate()
    {
        auto global = get_global< my_global_t >();
        throw std::runtime_error("Not implemented");
    };
};

int main() {
	auto global = std::make_shared< my_global_t >();
	auto writer =  output_writer_t( "results.txt" );
    writer.write_line("2025-01-09-AntonioMario-RossiPatrizio-1234567");

    my_input_parser_t parser( global );
    parser.parse();

    auto system = system_t::create( global, "my_system" );
    auto process = process_t::create();
    process->add_thread( std::make_shared< my_thread_t >() );
    system->add_process( process );

    // auto simulator = std::make_shared< my_simulator_t >( system );
    // auto montecarlino = montecarlo_t::create( simulator );
    // montecarlino->run();

}
