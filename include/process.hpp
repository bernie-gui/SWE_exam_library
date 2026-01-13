/*
 * File: process.hpp
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
 *	This header file defines the process_t and thread_t classes for managing processes and threads in the simulation
 * framework.
 */
#pragma once
#include <cassert>
#include <memory>
#include <optional>
#include <vector>
#include "common.hpp"
#include "network/message.hpp"
#include "system.hpp"
// process_t + thread_t

namespace isw
{
    class thread_t;
    /**
     * @brief Represents a process in the simulation system, managing a collection of threads.
     * @details Handles thread scheduling, registration with the system, and provides access to system resources.
     */
    class process_t : public std::enable_shared_from_this< process_t >
    {
        friend class thread_t;

    public:
        /**
         * @brief Constructor.
         * @param[in] name Name of the process, defaults to "default_process".
         * @details Registers the process with the system if available.
         */
        process_t( std::string name = "default_process" );
        ~process_t() = default;
        /*
         * schedule process threads at current_time
         */
        /**
         * @brief Schedules all threads at the current time.
         * @param[in] current_time The current simulation time.
         * @details Shuffles the threads randomly and calls schedule on each.
         */
        void schedule( double current_time );
        /**
         * @brief Gets the associated system.
         * @return Shared pointer to the system.
         */
        std::shared_ptr< system_t > get_system() const;
        /**
         * @brief Returns the minimum thread time among all threads.
         * @return The next update time, or infinity if no threads.
         */
        double next_update_time() const;
        /**
         * @brief Adds a thread to the process.
         * @param[in] thread Shared pointer to the thread.
         * @return Shared pointer to this process for chaining.
         * @details Sets the process pointer in the thread.
         */
        std::shared_ptr< process_t > add_thread( std::shared_ptr< thread_t > thread );
        /**
         * @brief Sets the process ID.
         * @param[in] id The ID to set.
         */
        void set_id( size_t id, std::optional< world_key_t > world = std::nullopt,
                     std::optional< size_t > relative_id = std::nullopt );
        /**
         * @brief Gets the process ID.
         * @return Optional containing the ID if set.
         */
        std::optional< size_t > get_id() const;

        std::optional< size_t > get_relative_id() const;
        std::optional< world_key_t > get_world_key() const;

        /**
         * @brief Initializes the process and its threads.
         * @details Calls init on all threads.
         */
        virtual void init();

        /**
         * @brief Gets the global state, cast to type T.
         * @tparam T Type to cast to, defaults to global_t.
         * @return Shared pointer to the casted global.
         */
        template< typename T = global_t >
        std::shared_ptr< T > get_global()
        {
            auto casted = std::dynamic_pointer_cast< T >( get_system()->get_global() );
            assert( casted.get() != nullptr );
            return casted;
        }

        /**
         * @brief Sets the system for this process.
         * @param[in] system Shared pointer to the system.
         */
        void set_system( std::shared_ptr< system_t > system );

        /**
         * @brief Gets the active state of the process.
         * @return The active state of the process.
         */
        bool is_active() const;
        /**
         * @brief Sets the active state of the process.
         * @param[in] active The active state to set.
         */
        void set_active( bool active );

        /**
         * @brief Factory method to create a process.
         * @param[in] name Name of the process.
         * @return Shared pointer to the created process.
         */
        static std::shared_ptr< process_t > create( std::string name = "default_process" );

    private:
        /** @brief Optional process ID for system identification. */
        std::optional< size_t > _id;
        /** @brief Optional process Relative ID for system identification. */
        std::optional< size_t > _relative_id;
        /** @brief Optional process World Key for system identification. */
        std::optional< world_key_t > _world_key;
        /** @brief The associated system. */
        std::shared_ptr< system_t > _system;
        /** @brief List of threads in this process. */
        std::vector< std::shared_ptr< thread_t > > _threads;
        /** @brief Name of the process. */
        std::string _name;
        /** @brief Deactivation Flag */
        bool _is_active;
    };

    /**
     * @brief Base class for threads in the simulation system.
     * @details Manages timing, scheduling, and message passing.
     */
    class thread_t
    {
    public:
        /**
         * @brief Constructor.
         * @param[in] c_time Compute time, defaults to 0.0.
         * @param[in] s_time Sleep time, defaults to 0.0.
         * @param[in] th_time Thread time, defaults to 0.0.
         */
        thread_t( double c_time = 0.0, double s_time = 0.0, double th_time = 0.0 );


        /**
         * @brief Sets the thread time.
         * @param[in] _th_time The thread time to set.
         */
        void set_thread_time( double _th_time );


        /**
         * @brief Sets the compute time.
         * @param[in] _c_time The compute time to set.
         */
        void set_compute_time( double _c_time );

        /**
         * @brief Sets the sleep time.
         * @param[in] _s_time The sleep time to set.
         */
        void set_sleep_time( double _s_time );

        /**
         * @brief Gets the thread time.
         * @return The current thread time.
         */
        double get_thread_time() const;
        /**
         * @brief Gets the compute time.
         * @return The compute time.
         */
        double get_compute_time() const;
        /**
         * @brief Gets the sleep time.
         * @return The sleep time.
         */
        double get_sleep_time() const;
        /**
         * @brief Gets the parent process.
         * @return Shared pointer to the process.
         */
        template< typename T = process_t >
        std::shared_ptr< T > get_process() const
        {
            auto casted = std::dynamic_pointer_cast< T >( _process );
            assert( casted.get() != nullptr );
            return casted;
        }

        /**
         * @brief Schedules the thread if its time has come.
         * @param[in] current_time Current simulation time.
         * @details If thread time <= current time, calls fun() and updates thread time.
         */
        void schedule( double current_time );
        /**
         * @brief Sets the parent process.
         * @param[in] process Shared pointer to the process.
         */
        void set_process( std::shared_ptr< process_t > process );

        /**
         * @brief Gets the thread active state.
         * @return The active state of the thread.
         */
        bool is_active() const;
        /**
         * @brief Sets the active state of the process.
         * @param[in] active The active state to set.
         */
        void set_active( bool active );


        /**
         * @brief Sends a message to another process.
         * @tparam T Message type, must derive from message_t.
         * @param[in] receiver_id ID of the receiving process.
         * @param[in,out] msg The message to send.
         * @details Sets timestamp, sender, receiver, and sends via system.
         */
        template< typename T, typename = std::enable_if_t< std::is_base_of_v< isw::network::message_t, T > > >
        void send_message( std::size_t receiver_id, T &msg )
        {
            const std::shared_ptr< process_t > process = this->get_process();
            assert( process->get_id().has_value() );

            const std::shared_ptr< system_t > system = process->get_system(); // questa
            assert( system.get() != nullptr );

            // set common base fields
            auto &base = static_cast< isw::network::message_t & >( msg );
            base.receiver = receiver_id;
            base.timestamp = system->get_current_time();
            base.sender = process->get_id().value();              // assert
            base.world_key = process->get_world_key().value();    // assert
            base.sender_rel = process->get_relative_id().value(); // assert

            system->send_message( std::make_shared< T >( std::move( msg ) ) );
        }

        /**
         * @brief Sends a message to a process in another world.
         * @tparam T Message type, must derive from message_t.
         * @param[in] world The world key.
         * @param[in] rel_id Relative ID in the world.
         * @param[in,out] msg The message to send.
         * @details Converts relative ID to absolute and sends the message.
         */
        template< typename T, typename = std::enable_if_t< std::is_base_of_v< isw::network::message_t, T > > >
        void send_message( world_key_t world, size_t rel_id, T &msg )
        {
            const std::shared_ptr< system_t > system = _process->get_system();
            send_message< T >( system->get_abs_id( world, rel_id ), msg );
        }

        /**
         * @brief Gets the global state, cast to type T.
         * @tparam T Type to cast to, defaults to global_t.
         * @return Shared pointer to the casted global.
         */
        template< typename T = global_t >
        std::shared_ptr< T > get_global()
        {
            auto casted = std::dynamic_pointer_cast< T >( get_process()->get_system()->get_global() );
            assert( casted.get() != nullptr );
            return casted;
        }

        /**
         * @brief Receives a message from the process's input queue.
         * @tparam T Message type, defaults to message_t.
         * @return Shared pointer to the received message, or nullptr if none.
         */
        template< typename T = network::message_t >
        std::shared_ptr< T > receive_message()
        {
            assert( _process.get() != nullptr ); // MAKE SURE THIS THREAD IS ASSOCIATED TO A PROCESS
            auto proc_id = _process->get_id();
            assert( proc_id.has_value() );       // MAKE SURE THIS THREAD'S PROCESS IS REGISTERD IN THE SYSTEM
            auto global = _process->get_system()->get_global();

            auto &queue = global->get_channel_in()[proc_id.value()];
            if ( queue.empty() )
                return nullptr;

            auto front_msg = queue.front();
            queue.pop();
            // // todo asset
            assert( front_msg->receiver == proc_id.value() );
            // JUST TO BE SURE INSTEAD OF A DESTRUCTIVE ASSERT
            // IF RECEIVER IS WRONG SOMETHING WENT REALLY WRONG IN THE LIBRARY
            return std::dynamic_pointer_cast< T >( front_msg );
        }
        /**
         * @brief Pure virtual function to be implemented by subclasses.
         * @details Contains the thread's logic.
         */
        virtual void fun() = 0;
        /**
         * @brief Initializes the thread's timing to initial values.
         */
        virtual void init();

    private:
        /** @brief Current thread time. */
        double _th_time;
        /** @brief Compute time. */
        double _c_time;
        /** @brief Sleep time. */
        double _s_time;
        /** @brief Initial thread time. */
        double _initial_th_time;
        /** @brief Initial compute time. */
        double _initial_c_time;
        /** @brief Initial sleep time. */
        double _initial_s_time;
        /** @brief Parent process. */
        std::shared_ptr< process_t > _process;
        /** @brief Deactivation Flag */
        bool _is_active; // assume spherical cow
    };
} // namespace isw
