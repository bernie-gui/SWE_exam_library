#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <memory>
#include <queue>
#include <sstream>
#include <stdexcept>

#include "global.hpp"
#include "montecarlo.hpp"
#include "network/network.hpp"
#include "process.hpp"
#include "simulator.hpp"
#include "system.hpp"


#include "io/input_parser.hpp"
#include "io/output_writer.hpp"


using namespace isw;


class my_global_t : public global_t
{
public:
    my_global_t() : global_t() {}

    double _horizon;
    void set_horizon( double horizon ) { this->_horizon = horizon; }
    double get_horizon() { return _horizon; }


    double time_step;
    size_t num_of_products;

    double min_customer_wait_time;
    double max_customer_wait_time;
    double prob_of_refill;
    size_t num_of_servers;
    size_t time_of_server_serve;
    size_t time_of_server_refill;
    size_t producst_initial_fill_value = 50;
    size_t producst_refill_value = 50;

    double missed_sell_rate = 0;
    double missed_rate_events = 0;
};

class my_input_parser_t : public input_parser_t
{
public:
    my_input_parser_t( std::shared_ptr< global_t > global ) : input_parser_t( global, "parameters.txt" ) {}

    size_t n_states;
    void parse() override
    {
        auto &stream = get_stream();


        char tag;

        std::string line;


        auto global = get_global< my_global_t >();

        while ( std::getline( stream, line ) )
        {
            std::istringstream ss( line );

            ss >> tag;

            switch ( tag )
            {
                case 'H':
                    double horzion;
                    ss >> horzion;
                    global->set_horizon( horzion );
                    break;

                case 'T':
                    ss >> global->time_step;
                    break;
                case 'M':
                    double val;
                    ss >> val;
                    global->set_montecarlo_budget( val );
                    break;
                case 'N':
                    size_t val2;
                    ss >> val2;
                    global->num_of_products = val2;
                    break;
                case 'A':
                    size_t val3;
                    ss >> val3;
                    global->min_customer_wait_time = val3;
                    break;
                case 'B':
                    size_t val4;
                    ss >> val4;
                    global->min_customer_wait_time = val4;
                    break;
                case 'p':
                    size_t val5;
                    ss >> val5;
                    global->prob_of_refill = val5;
                    break;
                case 'S':
                    size_t val6;
                    ss >> val6;
                    global->num_of_servers = val6;
                    break;
                case 'F':
                    size_t val7;
                    ss >> val7;
                    global->time_of_server_serve = val7;
                    break;
                case 'G':
                    size_t val8;
                    ss >> val8;
                    global->time_of_server_refill = val8;
                    break;
                case 'K':
                    size_t val9;
                    ss >> val9;
                    global->producst_initial_fill_value = val9;
                    break;
                case 'Q':
                    size_t val10;
                    ss >> val10;
                    global->producst_refill_value = val10;
                    break;
            }
        }
    }
};

class my_message : public isw::network::message_t
{
public:
    my_message() = default;

    size_t item_id;
};


class customer_t : public process_t
{
public:
    customer_t( std::string name ) : process_t( name ) {}

    static std::shared_ptr< customer_t > create( std::string name = "default_process" )
    {
        return std::make_shared< customer_t >( name );
    }
};

class customer_thread_t : public thread_t
{
public:
    customer_thread_t() : thread_t( .1, .1, 0 ) {}
    void init() override { update_times(); }

    void update_times()
    {
        auto global = get_global< my_global_t >();
        auto random = global->get_random();

        set_sleep_time( random->uniform_range( global->min_customer_wait_time, global->max_customer_wait_time ) );
    }

    void fun() override
    {
        auto global = get_global< my_global_t >();
        auto random = global->get_random();

        auto process = get_process< customer_t >();
        auto system = process->get_system();

        size_t num_of_servers = system->world_size( "server" );
        size_t random_server_id = random->uniform_range( 0, num_of_servers - 1 );

        my_message msg = my_message();
        size_t random_item_id = random->uniform_range( 0, global->num_of_products - 1 );

        msg.item_id = random_item_id;

        send_message< my_message >( "server", random_server_id, msg );

        update_times();
    }
};


class server_t : public process_t
{
public:
    server_t( std::string name ) : process_t( name ) {}

    static std::shared_ptr< server_t > create( std::string name = "default_process" )
    {
        return std::make_shared< server_t >( name );
    }

    void init() override
    {
        process_t::init();
        auto global = get_global< my_global_t >();
        auto random = get_global< my_global_t >()->get_random();

        items.resize( global->num_of_products );
        for ( size_t i = 0; i < global->num_of_products; i++ )
        {

            items[i] = random->uniform_range( 0, global->producst_initial_fill_value );
        }
    }
    std::vector< size_t > items;
    std::queue< size_t > refill_queue;
};

class refill_thread : public thread_t
{
public:
    refill_thread( double s_time = 0.1 ) : thread_t( 0.0, s_time, 0.0 ) {}

    void fun() override
    {
        auto pro = get_process< server_t >();
        auto global = get_global< my_global_t >();
        auto random = get_global()->get_random();

        while ( not pro->refill_queue.empty() )
        {
            size_t item = pro->refill_queue.front();
            pro->refill_queue.pop();

            pro->items[item] += random->uniform_range( 0, global->producst_refill_value );
        }
    }
};


class serve_thread : public thread_t
{
public:
    serve_thread( double s_time = 0.1 ) : thread_t( 0.0, s_time, 0.0 ) {}

    void fun() override
    {
        auto pro = get_process< server_t >();
        auto global = get_global< my_global_t >();
        auto random = get_global()->get_random();

        auto request_msg = this->receive_message< my_message >();
        if ( request_msg == nullptr )
        {
            return;
        }

        size_t item_id = request_msg->item_id;

        my_message resposne_msg;
        int missed_sell = 0;


        if ( pro->items[item_id] > 0 )
        {
            resposne_msg.item_id = item_id;
            pro->items[item_id]--;
        }
        else
        {
            resposne_msg.item_id = -item_id;

            double prob = random->uniform_range( 0.0, 1.0 );

            if ( prob < global->prob_of_refill )
            {
                pro->refill_queue.push( item_id );
            }
            missed_sell = 1;
        }

        global->missed_rate_events++; // MUST exist

        global->missed_sell_rate =
            global->missed_sell_rate * ( ( global->missed_rate_events - 1 ) / global->missed_rate_events ) +
            missed_sell / global->missed_rate_events;



        send_message( request_msg->sender, resposne_msg );
    }
};


class my_simulator_t : public simulator_t
{
public:
    using simulator_t::simulator_t;

    virtual bool should_terminate() override
    {
        auto global = get_global< my_global_t >();
        auto system = get_system();

        return system->get_current_time() >= global->get_horizon();
    };

    void on_terminate() override
    {
        auto global = get_global< my_global_t >();
        global->set_montecarlo_current( global->missed_sell_rate );
    }
};

int main()
{
    auto global = std::make_shared< my_global_t >();
    auto writer = output_writer_t( "results.txt" );
    writer.write_line( "2025-01-09-AntonioMario-RossiPatrizio-1234567\n" );

    my_input_parser_t parser( global );
    parser.parse();
    std::cout << global->get_horizon() << std::endl;

    auto system = system_t::create( global, "my_system" );

    auto customer = customer_t::create();
    customer->add_thread( std::make_shared< customer_thread_t >() );
    system->add_process( customer, "customer" );

    for ( size_t i = 0; i < global->num_of_servers; i++ )
    {
        auto server = server_t::create();
        auto th1 = std::make_shared< refill_thread >( global->time_of_server_refill );
        auto th2 = std::make_shared< serve_thread >( global->time_of_server_serve );
        server->add_thread( th1 );
        server->add_thread( th2 );
        system->add_process( server, "server" );
    }

    system->add_network();


    auto simulator = std::make_shared< my_simulator_t >( system );
    auto montecarlino = montecarlo_t::create( simulator );
    montecarlino->run();

    std::cout << "V " << global->get_montecarlo_avg() << "\n";
}
