/*
 * File: system.hpp
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
 *	This header file defines the system_t class for managing the overall system including processes and network.
 */
#pragma once

#include <cstddef>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>
#include "common.hpp"
#include "global.hpp"
#include "network/message.hpp"

namespace isw
{
    struct world_entry_t
    {
        world_key_t world;
        size_t rel_id;
    };

    class process_t;
    class network_t;
    using process_ptr_t = std::shared_ptr< process_t >;
    /**
     * @brief Manages the overall simulation system including processes, networks, and worlds.
     * @details Coordinates processes and networks, handles message passing, and manages simulation time.
     */
    class system_t : public std::enable_shared_from_this< system_t >
    {
    public:
        /**
         * @brief Constructor.
         * @param[in] global Shared pointer to global state, defaults to new global_t.
         * @param[in] name System name, defaults to "default_system".
         */
        system_t( std::shared_ptr< global_t > global,
                  const std::string &name = "default_system" );
        /**
         * @brief Initializes the system.
         * @details Initializes global, processes, and networks, resets time to 0.
         */
        virtual void init();
        /**
         * @brief Advances the simulation by one step.
         * @details Updates time to the next event time, schedules processes and networks.
         */
        virtual void step();
        /**
         * @brief Called at the end of each step.
         * @details Virtual method for post-step actions, default does nothing.
         */
        virtual void on_end_step();
        /**
         * @brief Adds a default network with scanner thread.
         * @param[in] nc_time Compute time for scanner.
         * @param[in] ns_time Sleep time for scanner.
         * @param[in] nth_time Thread time for scanner.
         * @return Shared pointer to this system.
         */
        std::shared_ptr< system_t > add_network( double nc_time = 0.1, double ns_time = 0.1, double nth_time = 0 );
        /**
         * @brief Adds a custom network.
         * @param[in] net Shared pointer to the network.
         * @return Shared pointer to this system.
         */
        std::shared_ptr< system_t > add_network( std::shared_ptr< network_t > net );
        /**
         * @brief Registers a process to a specific world.
         * @param[in] p Shared pointer to the process.
         * @param[in] world World key, defaults to "default".
         * @return Shared pointer to this system.
         */
        std::shared_ptr< system_t > add_process( process_ptr_t p, world_key_t world = "default" );
        /**
         * @brief Retrieves the absolute ID of a process in a specific world.
         * @param[in] world World key.
         * @param[in] rel_id Relative ID in the world.
         * @return Absolute process ID.
         * @throws std::out_of_range If world not found or rel_id out of range.
         */
        size_t get_abs_id( world_key_t world, size_t rel_id ) const;
        /**
         * @brief Retrieves the absolute ID of a process in a specific world.
         * @param[in] abs_id Absolute ID
         * @return Return a tuple containing the world key and relative ID.
         * @throws std::out_of_range If abs_id is invalid.
         */
        world_entry_t get_rel_id( size_t abs_id ) const;

        /**
         * @brief Gets all processes in a specific world.
         * @param[in] world World key.
         * @return Vector of process pointers.
         * @throws std::out_of_range If world not found.
         */
        template< typename T = process_t >
        const std::vector< std::shared_ptr< T > >
        get_processes( std::optional< world_key_t > world = std::nullopt ) const
        {
            std::vector< std::shared_ptr< T > > procs;
            if ( world.has_value() )
            {
                auto it = _worlds.find( world.value() );
                if ( it == _worlds.end() )
                {
                    throw std::out_of_range( "world key not found, " + world.value() );
                }
                else
                {
                    auto &world_set = it->second;
                    for ( auto id : world_set )
                    {
                        auto casted = std::dynamic_pointer_cast< T >( _processes[id] );
                        if ( !casted )
                            continue;
                        procs.push_back( casted );
                    }
                }
            }
            else
            {
                for ( auto &proc : _processes )
                {
                    auto casted = std::dynamic_pointer_cast< T >( proc );
                    if ( !casted )
                        continue;
                    procs.push_back( casted );
                }
            }

            return procs;
        }

        /**
         * @brief Gets all processes in the system.
         * @return Reference to vector of all process pointers.
         */
        const std::vector< process_ptr_t > &get_processes() const;
        /**
         * @brief Gets the global state, cast to type T.
         * @tparam T Type to cast to, defaults to global_t.
         * @return Shared pointer to the casted global.
         */
        template< typename T = global_t >
        std::shared_ptr< T > get_global()
        {
            auto casted = std::dynamic_pointer_cast< T >( _global );
            assert( casted.get() != nullptr );
            return casted;
        }
        /**
         * @brief Gets the size of a world.
         * @param[in] world World key.
         * @return Number of processes in the world.
         * @throws std::out_of_range If world not found.
         */
        size_t world_size( world_key_t world ) const;
        /**
         * @brief Gets the total number of worlds.
         * @return Number of worlds.
         */
        size_t total_worlds() const;
        /**
         * @brief Sends a message to a process by absolute ID.
         * @param[in] msg Shared pointer to the message.
         * @details Pushes the message to the receiver's output channel.
         */
        void send_message( const std::shared_ptr< network::message_t > msg );
        /**
         * @brief Gets the current simulation time.
         * @return Current time.
         */
        double get_current_time() const;
        /**
         * @brief Factory method to create a system.
         * @param[in] global Shared pointer to global, defaults to new.
         * @param[in] name System name.
         * @return Shared pointer to created system.
         */
        static std::shared_ptr< system_t > create( std::shared_ptr< global_t > global = std::make_shared< global_t >(),
                                                   std::string name = "default_system" );

    private:
        /** @brief Current simulation time. */
        double _time;
        /** @brief List of all processes. */
        std::vector< process_ptr_t > _processes;
        /** @brief List of networks. */
        std::vector< std::shared_ptr< network_t > > _networks;
        /** @brief Map of worlds to sets of process IDs. */
        std::map< world_key_t, std::set< size_t > > _worlds;
        /** @brief Global state. */
        std::shared_ptr< global_t > _global;
        /** @brief System name. */
        const std::string _name;

        /** @brief Updates _time to the minimum next update time. */
        void _update_time();
    };
} // namespace isw
