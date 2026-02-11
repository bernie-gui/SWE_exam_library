/*
 * File: system.cpp
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
 *	This file implements the system_t class methods for managing the simulation system.
 */
#include "system.hpp"
#include <algorithm>
#include <cstddef>
#include <limits>
#include <memory>
#include <stdexcept>
#include "common.hpp"
#include "network/network.hpp"
#include "network/pid_network.hpp"

using namespace isw;

system_t::system_t( std::shared_ptr< global_t > global, const std::string &name ) : _global( global ), _name( name ) {}

void system_t::init()
{
    _global->init();
    for ( auto &process : _processes )
    {
    	process->set_active(true); //...
        process->init();
    }
    for ( auto &network : _networks )
    {
        network->init();
    }

    // reset time for run
    _time = 0;
}

std::shared_ptr< system_t > system_t::add_network( double nc_time, double ns_time, double nth_time )
{
    auto net = std::make_shared< network_t >();
    net->add_thread( std::make_shared< scanner_t >( nc_time, ns_time, nth_time ) );
    return add_network( net );
}

std::shared_ptr< system_t > system_t::add_network( std::shared_ptr< network_t > net )
{
    auto id = _networks.size();
    _networks.push_back( net );
    net->set_system( shared_from_this() );
    net->set_id( id );
    return shared_from_this();
}

std::shared_ptr< system_t > system_t::add_pid_network( double obj_occupancy, double th_time, double error_threshold ) {
    auto net = std::make_shared< network_t >();
    net->add_thread( std::make_shared< pid_scanner_t >( obj_occupancy, th_time, error_threshold ) );
    return add_network(net);
}

void system_t::_update_time()
{
    double time = std::numeric_limits< double >::infinity();
    for ( auto &proc : _processes ) {
   		if (!proc->is_active())
    		continue;
        time = std::min< double >( proc->next_update_time(), time );
    }
    for ( auto &net : _networks )
        time = std::min< double >( net->next_update_time(), time );
    _time = time;
}

void system_t::step()
{
    _update_time();
    auto shuffled = _processes;
    std::shuffle( shuffled.begin(), shuffled.end(), _global->get_random()->get_engine() );
    for ( auto &proc : shuffled ) {
	   	if (!proc->is_active())
	    		continue;
        proc->schedule( _time );
    }
    for ( auto &net : _networks )
        net->schedule( _time );
    on_end_step();
}

void system_t::on_end_step() {};

std::shared_ptr< system_t > system_t::add_process( process_ptr_t p, world_key_t world_key )
{

    const auto id = _processes.size();
    _processes.push_back( p );
    const auto rel_id = _worlds[world_key].size();
    _worlds[world_key].insert( id ); // assert
    const auto shared = this->shared_from_this();
    p->set_system( shared );
    p->set_id( id, world_key, rel_id );

    auto &in = _global->get_channel_in();
    auto &out = _global->get_channel_out();
    out.resize( _processes.size() );
    in.resize( _processes.size() );
    return shared_from_this();
}

size_t system_t::get_abs_id( world_key_t world, size_t rel_id ) const
{
    auto it = _worlds.find( world );
    if ( it == _worlds.end() )
    {
        throw std::out_of_range( "world key not found" );
    }
    else
    {
        auto &world_set = it->second;
        if ( rel_id >= world_set.size() )
            throw std::out_of_range( "relative ID out of range" );
        auto it = world_set.begin();
        std::advance( it, rel_id );
        return *it;
    }
}

world_entry_t system_t::get_rel_id( size_t abs_id ) const {
	// iterator worlds to find the world containing abs_id
	for (auto& [world_key, world] : _worlds) {
		if (world.find(abs_id) != world.end()) {
			// found the world
			// now find the relative id
			size_t rel_id = 0;
			for (auto id : world) {
				if (id == abs_id) {
					return {world_key, rel_id};
				}
				rel_id++;
			}
		}
	}
	throw std::out_of_range("absolute ID not found" );
}

const std::vector< process_ptr_t > &system_t::get_processes() const { return _processes; }

double system_t::get_current_time() const { return _time; }


std::shared_ptr< system_t > system_t::create( std::shared_ptr< global_t > global, std::string name )
{
    return std::make_shared< system_t >( global, name );
}

size_t system_t::world_size( world_key_t world ) const
{
    auto it = _worlds.find( world );
    if ( it == _worlds.end() )
    {
        throw std::out_of_range( "world key not found" );
        return 0;
    }
    return it->second.size();
}

size_t system_t::total_worlds() const { return _worlds.size(); };

void system_t::send_message( std::shared_ptr< network::message_t > msg )
{
    auto &out = _global->get_channel_out();
    out[msg->sender].push( msg );
}
