/*
 * File: process.cpp
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
 *	This file implements the process_t class methods for process management in the simulation system.
 */
#include "process.hpp"
#include <algorithm>
#include <cassert>
#include <memory>
#include <optional>
#include "common.hpp"
#include "system.hpp"
using namespace isw;

process_t::process_t( std::string name ) : _id( std::nullopt ), _name( name ), _is_active( true ) {}

void process_t::init()
{
    for ( auto &thread : this->_threads )
    {
        thread->init();
    }
};


void process_t::schedule( double current_time )
{
    assert( _system.get() != nullptr ); // ENSURE THIS PROCESS IS ASSOCIATED TO A SYSTEM
    auto random = _system->get_global()->get_random();
    auto shuffled = _threads;
    std::shuffle( shuffled.begin(), shuffled.end(), random->get_engine() );
    for ( auto &thread : shuffled )
        thread->schedule( current_time );
}

std::shared_ptr< system_t > process_t::get_system() const { return _system; }

double process_t::next_update_time() const
{
    auto it = std::min_element( _threads.begin(), _threads.end(), []( const auto &a, const auto &b )
                                { return a->get_thread_time() < b->get_thread_time(); } );
    if ( it != _threads.end() )
    {
        return ( *it )->get_thread_time();
    }
    return std::numeric_limits< double >::infinity();
}

std::shared_ptr< process_t > process_t::add_thread( std::shared_ptr< thread_t > thread_ptr )
{
    _threads.push_back( thread_ptr );
    thread_ptr->set_process( this->shared_from_this() );
    return this->shared_from_this();
}

void process_t::set_id( size_t id, std::optional< world_key_t > world, std::optional< size_t > relative_id )
{
    _world_key = world;
    _relative_id = relative_id;
    _id = id;
}

void process_t::set_system( std::shared_ptr< system_t > system ) { _system = system; }

std::shared_ptr< process_t > process_t::create( std::string name ) { return std::make_shared< process_t >( name ); }

std::optional< size_t > process_t::get_id() const { return _id; }

bool process_t::is_active() const { return _is_active; }

void process_t::set_active( bool active )
{
    this->_is_active = active;
    if ( active && _system )
    {
        for ( auto &thread : _threads )
        {
            if ( !thread->is_active() )
                continue;
            thread->set_active( true );
        }
    }
}

thread_t::thread_t( double compute_time, double sleep_time, double thread_time ) :
    _th_time( thread_time ), _c_time( compute_time ), _s_time( sleep_time ), _initial_th_time( thread_time ),
    _initial_c_time( compute_time ), _initial_s_time( sleep_time ), _is_active( true ) {};


void thread_t::init()
{
    set_thread_time( _initial_th_time );
    set_compute_time( _initial_c_time );
    set_sleep_time( _initial_s_time );
}

double thread_t::get_thread_time() const { return _th_time; }
double thread_t::get_compute_time() const { return _c_time; }
double thread_t::get_sleep_time() const { return _s_time; }

void thread_t::set_thread_time( double _t_time ) { this->_th_time = _t_time; };
void thread_t::set_compute_time( double _c_time ) { this->_c_time = _c_time; }
void thread_t::set_sleep_time( double _s_time ) { this->_s_time = _s_time; };

void thread_t::schedule( double current_time )
{
    if ( this->_th_time > current_time )
        return;
    fun();
    _th_time += _c_time + _s_time;
}

void thread_t::set_process( std::shared_ptr< process_t > process ) { _process = process; }


std::optional< size_t > process_t::get_relative_id() const { return this->_relative_id; };

std::optional< world_key_t > process_t::get_world_key() const { return this->_world_key; };


bool thread_t::is_active() const { return _is_active; }

void thread_t::set_active( bool active )
{
    this->_is_active = active;
    if ( _process && active )
    {
        auto system = _process->get_system();
        if ( !system )
            return;
        _th_time = system->get_current_time();
    }
}
