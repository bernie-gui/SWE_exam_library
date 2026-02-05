/*
 * File: master.hpp
 * Copyright (c) 2025 bernie_gui, uniquadev, SepeFr.
 *
 * This file is part of SWE_exam_library used to generate amalgamated builds.
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
#pragma once

// Core components

/*** Start of inlined file: common.hpp ***/
#pragma once

#include <cstdint>
#include <string>
/*
 * TYPES DEFINITIONs
 */
/** @brief 8-bit unsigned integer type. */
typedef uint8_t u8_t;
/** @brief 16-bit unsigned integer type. */
typedef uint16_t u16_t;
/** @brief 32-bit unsigned integer type. */
typedef uint32_t u32_t;
/** @brief 64-bit unsigned integer type. */
typedef uint64_t u64_t;

namespace isw
{
    /** @brief Type for world keys, represented as strings. */
    using world_key_t = std::string;
} // namespace isw

/*** End of inlined file: common.hpp ***/



/*** Start of inlined file: global.hpp ***/
#pragma once

#include <cstddef>

/*** Start of inlined file: message.hpp ***/
#pragma once
#include <memory>
#include <queue>

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
        world_key_t world_key;        	/**< @brief Key identifying the world context of the message. */
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

/*** End of inlined file: message.hpp ***/



/*** Start of inlined file: random.hpp ***/
#pragma once

#include <random>

namespace isw
{
    /**
     * @brief Provides random number generation using various distributions.
     * @details Uses Mersenne Twister engine for generating uniform and Gaussian random numbers.
     */
    class random_t
    {
    public:
        /**
         * @brief Constructor with specified seed.
         * @param[in] seed The seed for the random engine.
         */
        random_t( size_t seed );
        /**
         * @brief Default constructor using random device for seed.
         */
        random_t();

        /**
         * @brief Generates a uniform random integer in range [min, max].
         * @param[in] min Minimum value.
         * @param[in] max Maximum value.
         * @return Random integer.
         */
        int uniform_range( int min, int max );
        /**
         * @brief Generates a uniform random double in range [min, max).
         * @param[in] min Minimum value.
         * @param[in] max Maximum value.
         * @return Random double.
         */
        double uniform_range( double min, double max );
        /**
         * @brief Generates a Gaussian random sample.
         * @param[in] mean Mean of the distribution.
         * @param[in] stddev Standard deviation.
         * @return Random sample from normal distribution.
         */
        double gaussian_sample( double mean, double stddev );

        /**
         * @brief Gets reference to the random engine.
         * @return Reference to the Mersenne Twister engine.
         */
        std::mt19937_64 &get_engine();

    private:
        /** @brief Mersenne Twister random engine. */
        std::mt19937_64 _engine;
    };

} // namespace isw

/*** End of inlined file: random.hpp ***/

namespace isw
{
    /**
     * @brief Manages global state and configuration for the simulation system.
     * @details Holds random number generator, message channels, and simulation parameters like budgets and results.
     */
    class global_t
    {
    public:
        /**
         * @brief Default constructor.
         * @details Initializes the random number generator.
         */
        global_t();
        virtual ~global_t() = default;
        global_t( const global_t &other ) noexcept = default;
        /**
         * @brief Initializes simulation-specific state.
         * @details Clears all message channels and resets montecarlo current to 0.
         */
        virtual void init();
        /**
         * @brief Returns the shared pointer to the random number generator.
         * @return Shared pointer to random_t.
         */
        std::shared_ptr< random_t > get_random();
        /**
         * @brief Returns reference to the input message channels.
         * @return Reference to vector of input channels.
         * @note The returned reference can be modified in-place.
         */
        std::vector< network::channel_t > &get_channel_in();
        /**
         * @brief Returns reference to the output message channels.
         * @return Reference to vector of output channels.
         * @note The returned reference can be modified in-place.
         */
        std::vector< network::channel_t > &get_channel_out();
        /*
         * simulation parameters getters/setters
         */
        /**
         * @brief Gets the simulation horizon.
         * @return The horizon value.
         */
        double get_horizon() const;
        /**
         * @brief Sets the simulation horizon.
         * @param[in] horizon The horizon value to set.
         */
        void set_horizon( double horizon );

        /**
         * @brief Gets the Monte Carlo budget.
         * @return The Monte Carlo budget value.
         */
        size_t montecarlo_budget() const;
        /**
         * @brief Sets the Monte Carlo budget.
         * @param[in] montecarlo_budget The budget value to set.
         */
        void set_montecarlo_budget( size_t montecarlo_budget );

        /**
         * @brief Gets the optimizer budget.
         * @return The optimizer budget value.
         */
        size_t optimizer_budget() const;
        /**
         * @brief Sets the optimizer budget.
         * @param[in] optimizer_budget The budget value to set.
         */
        void set_optimizer_budget( size_t optimizer_budget );

        /**
         * @brief Gets the network number.
         * @return The network number value.
         */
        size_t network_number() const;
        /**
         * @brief Sets the network number.
         * @param[in] network_number The network number to set.
         */
        void set_network_number( size_t network_number );

        /**
         * @brief Gets the Monte Carlo average.
         * @return The Monte Carlo average value.
         */
        double get_montecarlo_avg() const;
        /**
         * @brief Sets the Monte Carlo average.
         * @param[in] avg The average value to set.
         */
        void set_montecarlo_avg( double avg );

        /**
         * @brief Gets the current Monte Carlo value.
         * @return The current Monte Carlo value.
         */
        double montecarlo_current() const;
        /**
         * @brief Sets the current Monte Carlo value.
         * @param[in] current The current value to set.
         */
        void set_montecarlo_current( double current );

        /**
         * @brief Gets the optimizer result.
         * @return The optimizer result value.
         */
        double get_optimizer_result() const;
        /**
         * @brief Sets the optimizer result.
         * @param[in] current The result value to set.
         */
        void set_optimizer_result( double current );

        std::vector< double > get_optimizer_optimal_parameters() const;
        void set_optimizer_optimal_parameters( std::vector< double > current );

        // WARNING: TO CREATE MONITOR METHODS MAKE SURE TO IMPLEMENT THEM VIA EXTENDING THIS CLASS
        /*
               ⢀⡞⠹⣦⠰⡞⠙⣆
               ⢴  ⣿⠐⡇ ⢻
               ⢸  ⣿⢸⡇ ⢸
              ⢀⡸⣄ ⣿⣨⡇ ⣟⡀
           ⢠⡶⠚⠉⢁⡀     ⡈⠉⠙⠲⣤⡀
         ⢀⡶⠋ ⢀⠔⠉      ⠈⠑⢄ ⠈⠻⡄
         ⣾⠁  ⠈ ⣠⣂⡄   ⣔⣢ ⠈   ⢹
         ⡇  ⢠⣠⣠⡌⠓⠁ ⡀ ⠙⠊⡄⢀⣀  ⢸
         ⢷⡀ ⠈⠁⠁  ⠈⠓⡓⠂  ⠉⠈⠁  ⡼
         ⠈⠳⣄⡀             ⣠⡞⠁
           ⢾ ⡄           ⣴⢸
           ⠈⢻⡄           ⡾⠊⠁
            ⠘⣇⢀⡀        ⡀⣷
             ⢿⣼⠉⠉⠙⠛⠛⠛⠛⠉⢹⣁⠟

         */

    private:
        /** @brief Random number generator. */
        std::shared_ptr< random_t > _rand;
        /** @brief Input message channels for each process. */
        std::vector< network::channel_t > _channel_in;
        /** @brief Output message channels for each process. */
        std::vector< network::channel_t > _channel_out;
        /** @brief Simulation horizon. */
        double _horizon;
        /** @brief Monte Carlo budget. */
        size_t _montecarlo_budget;
        /** @brief Optimizer budget. */
        size_t _optimizer_budget;
        /** @brief Optimizer alphas */
        std::vector< double > _optimizer_optimal_parameters;
        /** @brief Network number. */
        size_t _network_number;
        /** @brief Monte Carlo average. */
        double _montecarlo_avg;
        /** @brief Current Monte Carlo value. */
        double _montecarlo_current;
        /** @brief Optimizer result. */
        double _optimizer_result;
        /** @brief Arguments vector. */
        std::vector< double > arguments;
    }; //
} // namespace isw

/*** End of inlined file: global.hpp ***/


/*** Start of inlined file: process.hpp ***/
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


/*** Start of inlined file: system.hpp ***/
#pragma once

#include <cstddef>
#include <unordered_map>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

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
        std::unordered_map< world_key_t, std::set< size_t > > _worlds;
        /** @brief Global state. */
        std::shared_ptr< global_t > _global;
        /** @brief System name. */
        const std::string _name;

        /** @brief Updates _time to the minimum next update time. */
        void _update_time();
    };
} // namespace isw

/*** End of inlined file: system.hpp ***/

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

/*** End of inlined file: process.hpp ***/


/*** Start of inlined file: simulator.hpp ***/
#pragma once

#include <cassert>
#include <memory>

/*** Start of inlined file: logger.hpp ***/
#pragma once
#include <filesystem>
#include <fstream>
#include <memory>
#include <string_view>
#include <vector>

namespace isw
{
    /**
     * @brief Logs simulation data to CSV files.
     * @details Manages field schema and measurement logging with chaining support.
     */
    class logger_t : public std::enable_shared_from_this< logger_t >
    {
    public:
        /**
         * @brief Factory method to create a logger.
         * @param[in] path Path to the log file, defaults to "logfile.csv".
         * @return Shared pointer to created logger.
         */
        static std::shared_ptr< logger_t > create( const std::filesystem::path &path = "logfile.csv" );
        /**
         * @brief Adds a field to the schema.
         * @param[in] field Field name.
         * @return Shared pointer to this logger for chaining.
         * @throws std::runtime_error If fields are modified after logging schema.
         */
        std::shared_ptr< logger_t > add_field( std::string_view field );
        /**
         * @brief Logs the field schema to the file.
         * @return Shared pointer to this logger for chaining.
         * @throws std::runtime_error If fields already logged.
         */
        std::shared_ptr< logger_t > log_fields();
        /**
         * @brief Adds a measurement value.
         * @param[in] value Measurement value.
         * @return Shared pointer to this logger for chaining.
         */
        std::shared_ptr< logger_t > add_measurement( std::string_view value );
        /**
         * @brief Logs the current measurements.
         * @return Shared pointer to this logger for chaining.
         * @throws std::runtime_error If measurement count doesn't match field count.
         */
        std::shared_ptr< logger_t > log_measurement();

    private:
        /** @brief Private constructor.
         * @param[in] path Path to the log file.
         * @throws std::runtime_error If file cannot be opened.
         */
        logger_t( const std::filesystem::path &path );
        /** @brief Flag to block field modifications after logging schema. */
        bool _block; /* = false */
                     /** @brief Output stream to the log file. */
        std::ofstream _stream;
        /** @brief List of field names. */
        std::vector< std::string > _fields;
        /** @brief List of current measurements. */
        std::vector< std::string > _measurements;
    };
} // namespace isw

/*** End of inlined file: logger.hpp ***/


namespace isw
{
    /**
     * @brief Base class for running simulations with termination conditions.
     * @details Manages the simulation loop, initializing the system and stepping until termination.
     */
    class simulator_t
    {
    public:
        /**
         * @brief Constructor.
         * @param[in] sys Shared pointer to the system to simulate.
         */
        simulator_t( const std::shared_ptr< system_t > sys );
        /**
         * @brief Runs the simulation.
         * @details Initializes the system, then steps until termination condition is met, then calls on_terminate.
         */
        virtual void run();
        /**
         * @brief Pure virtual termination condition.
         * @return True if simulation should terminate.
         * @details Must be overridden by subclasses to define when to stop the simulation.
         */
        virtual bool should_terminate();
        /**
         * @brief Called after simulation terminates.
         * @details Virtual method for cleanup or final actions, default does nothing.
         */
        virtual void on_terminate();

        /**
         * @brief Gets the system.
         * @return Shared pointer to the system.
         */
        std::shared_ptr< system_t > get_system();

        template< typename T = global_t >
        std::shared_ptr< T > get_global()
        {
            auto casted = std::dynamic_pointer_cast< T >( get_system()->get_global() );
            assert( casted );
            return casted;
        }

    private:
        /** @brief Logger instance (currently unused). */
        std::unique_ptr< logger_t > _logger;
        /** @brief The system being simulated. */
        std::shared_ptr< system_t > _system;
    };
} // namespace isw

/*** End of inlined file: simulator.hpp ***/

// Advanced features

/*** Start of inlined file: montecarlo.hpp ***/
#pragma once

#include <memory>

namespace isw
{
    /**
     * @brief Performs Monte Carlo simulations by running multiple simulation instances and averaging results.
     */
    class montecarlo_t
    {
    public:
        /**
         * @brief Runs the Monte Carlo simulation.
         * @details Initializes the average to 0, then runs the simulator for the budgeted number of times,
         * updating the running average of the Monte Carlo current values.
         */
        void run();
        /**
         * @brief Gets the simulator instance.
         * @return Shared pointer to the simulator.
         */
        std::shared_ptr< simulator_t > get_simulator() const;
        /**
         * @brief Factory method to create a montecarlo_t instance.
         * @param[in] sim Shared pointer to the simulator.
         * @return Shared pointer to the created montecarlo_t.
         */
        static std::shared_ptr< montecarlo_t > create( const std::shared_ptr< simulator_t > sim );

        template< typename SIM >
        /**
         * @brief Template factory method to create a montecarlo_t with a specific simulator type.
         * @tparam SIM The simulator type to create.
         * @param[in] sys Shared pointer to the system.
         * @return Shared pointer to the created montecarlo_t.
         */
        static std::shared_ptr< montecarlo_t > create( const std::shared_ptr< system_t > sys )
        {
            return std::shared_ptr< montecarlo_t >( new montecarlo_t( std::make_shared< SIM >( sys ) ) );
        }

    private:
        /** @brief Private constructor.
         * @param[in] sim Shared pointer to the simulator.
         */
        montecarlo_t( std::shared_ptr< simulator_t > sim );
        void _init();                        /**< @brief Initialization method (currently unused). */
        std::shared_ptr< simulator_t > _sim; /**< @brief The simulator instance. */
    };
} // namespace isw

/*** End of inlined file: montecarlo.hpp ***/



/*** Start of inlined file: optimizer.hpp ***/
/*
 * File: optimizer.hpp
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
 *	This header file defines the optimizer_t class and optimizer_strategy enum for optimization algorithms using Monte
 * Carlo methods.
 */
#pragma once
#include <cassert>
#include <memory>
#include <vector>

namespace isw
{
    /** @brief Enumeration for optimization strategies. */
    enum class optimizer_strategy
    {
        MINIMIZE, /**< @brief Minimize the objective function. */
        MAXIMIZE  /**< @brief Maximize the objective function. */
    };

    /**
     * @brief Abstract base class for optimization algorithms using Monte Carlo methods.
     * @details Provides framework for running optimization by sampling parameters and evaluating objective functions.
     */
    class optimizer_t
    {
    public:
        /**
         * @brief Constructor.
         * @param[in] montecarlo Shared pointer to the Monte Carlo simulator.
         */
        optimizer_t( std::shared_ptr< global_t > global );

        /*
         * This must run the simulation and make the calculations
         */
        /**
         * @brief Pure virtual objective function to be implemented by subclasses.
         * @param[in,out] arguments The parameter vector for evaluation.
         * @details This method must run the simulation and compute the objective value, storing it in global
         * optimizer_result.
         */
        virtual double obj_fun( std::vector< double > &arguments ) = 0;

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
         * @brief Runs the optimization process for multi-dimensional parameters.
         * @param[in] strategy The optimization strategy (MINIMIZE or MAXIMIZE).
         * @param[in] min_solution Vector of minimum bounds for each parameter.
         * @param[in] max_solution Vector of maximum bounds for each parameter.
         * @throws std::runtime_error If strategy is not implemented.
         * @details Samples random parameters within bounds, evaluates objective function, and tracks the best solution.
         */
        void optimize( optimizer_strategy strategy, std::vector< double > min_solution,
                       std::vector< double > max_solution );

        /**
         * @brief Runs the optimization process for single-dimensional parameter.
         * @param[in] strategy The optimization strategy.
         * @param[in] min_solution Minimum bound.
         * @param[in] max_solution Maximum bound.
         * @details Delegates to the vector version with single-element vectors.
         */
        void optimize( optimizer_strategy strategy, double min_solution, double max_solution );

    private:
        /** @brief The Monte Carlo simulator instance. */
        std::shared_ptr< global_t > _global;
    };

} // namespace isw

/*** End of inlined file: optimizer.hpp ***/

// IO components

/*** Start of inlined file: input_parser.hpp ***/
#pragma once

#include <cassert>
#include <filesystem>
#include <fstream>
#include <memory>

namespace isw
{

    /**
     * @brief Base class for parsing input files and configuring global state.
     * @details Provides stream access and global state management for input parsing.
     */
    class input_parser_t
    {
    public:
        /**
         * @brief Constructor.
         * @param[in] path Path to the input file.
         * @param[in] global Shared pointer to global state.
         * @throws std::runtime_error If file cannot be opened.
         */
        input_parser_t( std::shared_ptr< global_t > global,  const std::filesystem::path &path = "parameters.txt" );
        ~input_parser_t();

        // // forward extraction to the underlying stream
        // template< typename T >
        // input_parser_t &operator>>( T &value )
        // {
        //     _stream >> value; // uses ifstream::operator>>
        //     return *this; //  i love c++
        // }

        /**
         * @brief Pure virtual parse method.
         * @details Must be implemented by subclasses to parse the input file.
         */
        virtual void parse() = 0;
        /**
         * @brief Gets the input stream.
         * @return Reference to the ifstream.
         */
        std::ifstream &get_stream();
        /**
         * @brief Gets the global state, cast to type T.
         * @tparam T Type to cast to, defaults to global_t.
         * @return Shared pointer to casted global.
         */
        template< typename T = global_t >
        std::shared_ptr< T > get_global()
        {
            auto casted = std::dynamic_pointer_cast< T >( _global );
            assert( casted );
            return casted;
        }
        /**
         * @brief Resets the stream to the beginning.
         * @details Clears error flags and seeks to position 0.
         */
        void reset_stream();

    private:
        /** @brief Input file stream. */
        std::ifstream _stream;
        /** @brief Global state. */
        std::shared_ptr< global_t > _global;
    };
} // namespace isw

//    std::string line;
// while(std::getline(file, line))

// ifstream in;
// in >> whatever;
/*
 *jjjkk
 *
 */

/*** End of inlined file: input_parser.hpp ***/



/*** Start of inlined file: output_writer.hpp ***/
#pragma once

#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>

namespace isw
{

    /**
     * @brief Base class for writing simulation output to files.
     * @details Provides file stream and global state access for output writing.
     */
    class output_writer_t
    {
    public:
        /**
         * @brief Constructor.
         * @param[in] path Path to the output file.
         * @param[in] global Shared pointer to global state.
         * @throws std::runtime_error If file cannot be opened.
         */
        output_writer_t( const std::filesystem::path path = "results.txt" );
        ~output_writer_t(); // Destructor needed to properly close the file stream.

        /**
         * @brief Places the formatted line into the buffer (made using make_line)
         * @details The buffer must be later flushed with save_output
         */
        void write_line(const std::string &line);
        std::ofstream& get_stream();

        // Friend template for arbitrary types T
        template<typename T>
        friend output_writer_t& operator<<(output_writer_t& writer, const T& value) {
            writer.get_stream() << value;
            return writer;
        }

        // Special overloads for stream manipulators (e.g., std::endl)
        friend output_writer_t& operator<<(output_writer_t& writer, std::ostream& (*manip)(std::ostream&));
        friend output_writer_t& operator<<(output_writer_t& writer, std::ios_base& (*manip)(std::ios_base&));
    private:
        /** @brief Output stream to the file. */
        std::ofstream _stream;
    };

    /**
     * @brief Output stream manipulator overload (e.g., std::endl).
     */
    output_writer_t& operator<<(output_writer_t& writer, std::ostream& (*manip)(std::ostream&));

    /**
     * @brief Output stream manipulator overload.
     */
    output_writer_t& operator<<(output_writer_t& writer, std::ios_base& (*manip)(std::ios_base&));
} // namespace isw

/*** End of inlined file: output_writer.hpp ***/

// Network components


/*** Start of inlined file: network.hpp ***/
#pragma once
#include <vector>

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

/*** End of inlined file: network.hpp ***/

