#include <cmath>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "global.hpp"
#include "io/lambda_parser.hpp"
#include "io/output_writer.hpp"
#include "montecarlo.hpp"
#include "process.hpp"
#include "simulator.hpp"
#include "system.hpp"
#include "utils/customer-server/server.hpp"
#include "utils/customer-server/supplier.hpp"
#include "utils/customer-server/utils.hpp"
#include "utils/rate.hpp"

using namespace isw;
using namespace isw::cs;

class shop_global : public global_t
{
public:
    double T, H, M;
    size_t C, F, P, S, K;
    double A, B, V, Q; // Q is W

    utils::rate_meas_t measure;

    void init() override
    {
        global_t::init();
        measure.init();
    }
};

class customer_t : public process_t
{
public:
    customer_t( std::string name ) : process_t( name ) {}
    // Need a receiver thread to count missed sales
};

class customer_thread : public thread_t
{
public:
    customer_thread( double c_time ) : thread_t( c_time, 0, c_time ) {}

    void fun() override
    {
        auto gl = get_global< shop_global >();
        auto p = get_process< customer_t >();

        // Send logic
        // Chooses server s at random [0, S-1] (relative ID)
        // Chooses product i at random [0, P-1] (relative ID)
        // Sends i to s.

        // Wait... the text says "stays in state 0 for time tau... then sends...".
        // This implies the sending happens periodically.
        // My thread period is tau? No, tau is random [A, B].
        // I should set the NEXT update time based on [A, B].
        // thread_t has `set_compute_time`. `_c_time` is the period.
        // I can change `_c_time` dynamically.

        size_t s_idx = gl->get_random()->uniform_range( 0, gl->S - 1 );
        size_t product_idx = gl->get_random()->uniform_range( 0, gl->P - 1 );

        // We need to find the absolute ID of server s_idx
        // Servers are in world "servers"?
        auto sys = p->get_system();
        size_t server_id = sys->get_abs_id( "servers", s_idx );

        auto req = std::make_shared< request_t >();
        // The text says "sends value i".
        // The server receiving from customer checks if db[i] > 0.
        // So `item` = i.
        req->item = product_idx;
        if ( auto id = p->get_id() )
            req->sender = *id;
        req->receiver = server_id;
        req->world_key = "customers"; // sender world
        req->tag = 0;                 // generic tag

        sys->send_message( req );

        // Update next period
        double tau = gl->get_random()->uniform_range( gl->A, gl->B );
        set_compute_time( tau );
    }
};

class customer_receiver_thread : public thread_t
{
public:
    customer_receiver_thread() : thread_t( 0, 0, 0 ) {} // Continuous polling? Or small period?
    // Using 0 might be dangerous if it spins. Let's use a small period or rely on step?
    // Usually receiver threads have 0 wait if they just process inbox?
    // If I put 0, `step` calls `schedule`. `schedule` checks `_th_time`.
    // If I don't increase `_th_time`, it loops forever in one step? No, `step` calls `schedule` once per process per
    // step? No, `system::step` calls `proc->schedule(_time)`. `process::schedule` calls `thread->schedule`.
    // `thread::schedule` loops `while (_th_time <= current_time)`.
    // So if I don't advance time, infinite loop.
    // I should check mailbox. If empty, wait?
    // Better: check mailbox, if message, process. Then `set_compute_time` to something?
    // If I interpret "missed sales at time t", I process all messages arrived up to t.
    // I can just pop all messages.

    void fun() override
    {
        auto gl = get_global< shop_global >();
        while ( auto msg = receive_message< request_t >() )
        {
            // Text: "receives -j denotes missed sale".
            // Since `item` is size_t, how is -j represented?
            // Wait, message from server?
            // Server sends `j` or `-j`.
            // `request_t` has `size_t item`. It cannot hold negative values.
            // Maybe `request_t` isn't what the server sends back?
            // The server code in `example_11_25_1` (no server there).
            // `server.hpp` is generic on `mes_type`.
            // If I use `request_t`, I can interpret `tag` or `quantity` as the sign?
            // Or I can use `quantity` (int) for the signed value?
            // Let's assume the server sends a `request_t` back with `quantity` or `tag`.
            // The text says "sends message -j". Maybe `quantity = -j`? Or `item=j, quantity=-1`?
            // Let's check my server implementation plan.

            // I need to implement the server logic. I can use `request_t`.
            // If missed sale, I can set `quantity = -1`.
            // If sale, `quantity = 1`.

            if ( msg->quantity < 0 )
            {
                // Missed sale
                gl->measure.update( 1, get_thread_time() );
            }
        }
        // Check again next step?
        // Optimization: checking every T (0.1s) is too expensive with many customers.
        // We accumulate messages in the queue anyway.
        // Check every 10 seconds or H/10.
        set_compute_time( std::max( 10.0, gl->T * 100 ) );
    }
};

int main()
{
    auto gl = std::make_shared< shop_global >();

    lambda_parser_t parser( "examples/example_09_25_3.txt",
                            {
                                { "T", [gl]( auto &ss ) { ss >> gl->T; } },
                                { "H",
                                  [gl]( auto &ss )
                                  {
                                      ss >> gl->H;
                                      gl->set_horizon( gl->H );
                                  } },
                                { "M",
                                  [gl]( auto &ss )
                                  {
                                      ss >> gl->M;
                                      gl->set_montecarlo_budget( gl->M );
                                  } },
                                { "C", [gl]( auto &ss ) { ss >> gl->C; } },
                                { "A", [gl]( auto &ss ) { ss >> gl->A; } },
                                { "B", [gl]( auto &ss ) { ss >> gl->B; } },
                                { "F", [gl]( auto &ss ) { ss >> gl->F; } },
                                { "V", [gl]( auto &ss ) { ss >> gl->V; } },
                                { "Q", [gl]( auto &ss ) { ss >> gl->Q; } },
                                { "P", [gl]( auto &ss ) { ss >> gl->P; } },
                                { "S", [gl]( auto &ss ) { ss >> gl->S; } },
                                { "K", [gl]( auto &ss ) { ss >> gl->K; } },
                            } );
    parser.parse();

    auto sys = system_t::create( gl );

    // Servers
    // DB initialized random [0, K]
    cs::fill init_db = [gl]( size_t ) { return gl->get_random()->uniform_range( 0, gl->K ); };

    // Bindings for request_t
    // "customers" -> logic
    // "suppliers" -> logic

    cs::binding< request_t > bindings;

    bindings["customers"] = []( std::shared_ptr< server_thread_t< request_t > > th, std::shared_ptr< request_t > msg )
    {
        auto srv = std::dynamic_pointer_cast< server_t >( th->get_process() );
        size_t product = msg->item;

        // Reply message
        auto reply = std::make_shared< request_t >();
        if ( auto id = srv->get_id() )
            reply->sender = *id;
        reply->receiver = msg->sender;
        reply->world_key = "servers";
        reply->item = product;

        if ( srv->database[product] > 0 )
        {
            srv->database[product]--;
            // Send j (represented as quantity 1?)
            reply->quantity = 1;
        }
        else
        {
            // Send -j (represented as quantity -1?)
            reply->quantity = -1;
        }
        srv->get_system()->send_message( reply );
    };

    bindings["suppliers"] = []( std::shared_ptr< server_thread_t< request_t > > th, std::shared_ptr< request_t > msg )
    {
        auto srv = std::dynamic_pointer_cast< server_t >( th->get_process() );
        size_t product = msg->item;
        srv->database[product]++; // Increment by 1
    };

    for ( size_t i = 0; i < gl->S; ++i )
    {
        // Server thread parameters
        // c_time? Text doesn't specify server processing time. Assume 0 or small?
        // Usually 0 if instantaneous response.
        sys->add_process(
            server_t::create_process( gl->P, init_db, gl->T, bindings, 0, 0, 0, 0, "server_" + std::to_string( i ) ),
            "servers" );
    }

    // Suppliers
    // Supplier thread needs:
    // pick policy: random server
    cs::pick pick_policy = [gl]() { return gl->get_random()->uniform_range( 0, gl->S - 1 ); };
    // ask item: random item
    cs::ask ask_item = [gl]( size_t ) { return gl->get_random()->uniform_range( 0, gl->P - 1 ); };
    // ask quantity: 1
    cs::ask ask_quantity = []( size_t ) { return 1; };

    // Supplier timing: uniform [V, Q]
    // The supplier factory takes `c_time`.
    // But we need dynamic time.
    // Factory has `compute` std::function setter.
    cs::set supplier_compute = [gl]() { return gl->get_random()->uniform_range( gl->V, gl->Q ); };

    for ( size_t i = 0; i < gl->F; ++i )
    {
        // First period? Also random? The text says "stays in state 0 for time gamma... then sends".
        // So initial wait is also random.
        double init_wait = gl->get_random()->uniform_range( gl->V, gl->Q );
        // The factory takes `c_time` as initial?
        // `supplier_thread_t` constructor passes `c_time` to `thread_t`.
        // If we provide `compute` setter, `fun` calls it at the end.
        // But the FIRST run happens at `c_time`.
        // So we pass `init_wait` as `c_time`.

        sys->add_process( supplier_t::create_process( init_wait, pick_policy, ask_item, ask_quantity, "servers",
                                                      supplier_compute, 0, 0, 0, "supplier_" + std::to_string( i ) ),
                          "suppliers" );
    }

    // Customers
    for ( size_t i = 0; i < gl->C; ++i )
    {
        auto cust = std::make_shared< customer_t >( "customer_" + std::to_string( i ) );
        double init_wait = gl->get_random()->uniform_range( gl->A, gl->B );

        cust->add_thread( std::make_shared< customer_thread >( init_wait ) ); // Sender
        cust->add_thread( std::make_shared< customer_receiver_thread >() );   // Receiver

        sys->add_process( cust, "customers" );
    }

    class my_sim : public simulator_t
    {
    public:
        my_sim( std::shared_ptr< system_t > s ) : simulator_t( s ) {}
        void on_terminate() override
        {
            auto gl = get_global< shop_global >();
            gl->set_montecarlo_current( gl->measure.get_rate() );
        }
    };

    auto sim = std::make_shared< my_sim >( sys );
    auto monty = montecarlo_t::create( sim );

    // std::cout << "Starting Exam 3..." << std::endl;
    monty->run();
    // std::cout << "Exam 3 Ended." << std::endl;

    // output_writer_t out( "results_09_25_3.txt" );
    // out.write_line( "R " + std::to_string( gl->get_montecarlo_avg() ) );
    std::cout << "R " << gl->get_montecarlo_avg() << std::endl;

    return 0;
}
