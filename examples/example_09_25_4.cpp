#include <cmath>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "global.hpp"
#include "io/lambda_parser.hpp"
#include "montecarlo.hpp"
#include "optimizer.hpp"
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
    // Optimization budget G
    size_t G;

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
};

class customer_thread : public thread_t
{
public:
    customer_thread( double c_time ) : thread_t( c_time, 0, c_time ) {}

    void fun() override
    {
        auto gl = get_global< shop_global >();
        auto p = get_process< customer_t >();

        size_t s_idx = gl->get_random()->uniform_range( 0, gl->S - 1 );
        size_t product_idx = gl->get_random()->uniform_range( 0, gl->P - 1 );

        auto sys = p->get_system();
        size_t server_id = sys->get_abs_id( "servers", s_idx );

        auto req = std::make_shared< request_t >();
        req->item = product_idx;
        if ( auto id = p->get_id() )
            req->sender = *id;
        req->receiver = server_id;
        req->world_key = "customers";
        req->tag = 0;

        sys->send_message( req );

        double tau = gl->get_random()->uniform_range( gl->A, gl->B );
        set_compute_time( tau );
    }
};

class customer_receiver_thread : public thread_t
{
public:
    customer_receiver_thread() : thread_t( 0, 0, 0 ) {}

    void fun() override
    {
        auto gl = get_global< shop_global >();
        while ( auto msg = receive_message< request_t >() )
        {
            // std::cerr << "Cust: Received reply quantities=" << msg->quantity << " from " << msg->sender << std::endl;
            if ( msg->quantity < 0 )
            {
                // std::cerr << "Cust: Counting Missed Sale!" << std::endl;
                gl->measure.update( 1, get_thread_time() );
            }
        }
        // Optimization: check less frequently
        set_compute_time( std::max( 10.0, gl->T * 100 ) );
    }
};

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

class exam_optimizer : public optimizer_t< int >
{
public:
    exam_optimizer( std::shared_ptr< shop_global > gl ) : optimizer_t< int >( gl ) {}

    double obj_fun( std::vector< int > &arguments ) override
    {
        int F_current = arguments[0];
        if ( F_current < 1 )
            F_current = 1; // Minimum 1 supplier

        auto gl = get_global< shop_global >();
        // Temporarily set F to current candidate
        size_t original_F = gl->F;
        gl->F = static_cast< size_t >( F_current );

        // std::cerr << "Trying F=" << gl->F << std::endl;

        // Build system
        auto sys = std::make_shared< system_t >( gl );

        // Servers
        cs::fill init_db = [gl]( size_t ) { return gl->get_random()->uniform_range( 0, gl->K ); };
        cs::binding< request_t > bindings;

        bindings["customers"] =
            []( std::shared_ptr< server_thread_t< request_t > > th, std::shared_ptr< request_t > msg )
        {
            auto srv = std::dynamic_pointer_cast< server_t >( th->get_process() );
            size_t product = msg->item;
            auto reply = std::make_shared< request_t >();
            if ( auto id = srv->get_id() )
                reply->sender = *id;
            reply->receiver = msg->sender;
            reply->world_key = "servers";
            reply->item = product;

            if ( srv->database[product] > 0 )
            {
                srv->database[product]--;
                reply->quantity = 1;
                // std::cerr << "Srv: Sold item " << product << " to " << msg->sender << std::endl;
            }
            else
            {
                reply->quantity = -1;
                std::cerr << "Srv: Missed sale for " << product << " from " << msg->sender << std::endl;
            }
            srv->get_system()->send_message( reply );
        };

        bindings["suppliers"] =
            []( std::shared_ptr< server_thread_t< request_t > > th, std::shared_ptr< request_t > msg )
        {
            auto srv = std::dynamic_pointer_cast< server_t >( th->get_process() );
            size_t product = msg->item;
            srv->database[product]++;
        };

        for ( size_t i = 0; i < gl->S; ++i )
            sys->add_process( server_t::create_process( gl->P, init_db, 0.001, bindings, 0, 0, 0, 0,
                                                        "server_" + std::to_string( i ) ),
                              "servers" );

        // Suppliers
        cs::pick pick_policy = [gl]() { return gl->get_random()->uniform_range( 0, gl->S - 1 ); };
        cs::ask ask_item = [gl]( size_t ) { return gl->get_random()->uniform_range( 0, gl->P - 1 ); };
        cs::ask ask_quantity = []( size_t ) { return 1; };
        cs::set supplier_compute = [gl]() { return gl->get_random()->uniform_range( gl->V, gl->Q ); };

        for ( size_t i = 0; i < gl->F; ++i )
        {
            double init_wait = gl->get_random()->uniform_range( gl->V, gl->Q );
            sys->add_process( supplier_t::create_process( init_wait, pick_policy, ask_item, ask_quantity, "servers",
                                                          supplier_compute, 0, 0, 0, "sup_" + std::to_string( i ) ),
                              "suppliers" );
        }

        // Customers
        for ( size_t i = 0; i < gl->C; ++i )
        {
            auto cust = std::make_shared< customer_t >( "cust_" + std::to_string( i ) );
            double init_wait = gl->get_random()->uniform_range( gl->A, gl->B );
            cust->add_thread( std::make_shared< customer_thread >( init_wait ) );
            cust->add_thread( std::make_shared< customer_receiver_thread >() );
            sys->add_process( cust, "customers" );
        }

        auto sim = std::make_shared< my_sim >( sys );
        auto monty = montecarlo_t::create( sim );

        // Reset measure
        gl->measure.init();

        monty->run();

        double R = gl->get_montecarlo_avg();
        double J = 10.0 * F_current + 2.0 * R;

        // std::cerr << "End F=" << gl->F << " R=" << R << " J=" << J << std::endl;

        // Restore F? Not strictly needed as we rebuild system each time.
        // But good practice if gl is reused.
        gl->F = original_F;

        return J;
    }
};

int main()
{
    auto gl = std::make_shared< shop_global >();

    lambda_parser_t parser( "examples/example_09_25_4.txt",
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
                                { "G",
                                  [gl]( auto &ss )
                                  {
                                      ss >> gl->G;
                                      gl->set_optimizer_budget( gl->G );
                                  } },
                                { "V", [gl]( auto &ss ) { ss >> gl->V; } },
                                { "Q", [gl]( auto &ss ) { ss >> gl->Q; } },
                                { "P", [gl]( auto &ss ) { ss >> gl->P; } },
                                { "S", [gl]( auto &ss ) { ss >> gl->S; } },
                                { "K", [gl]( auto &ss ) { ss >> gl->K; } },
                            } );
    parser.parse();

    // std::cout << "Parsed G=" << gl->G << std::endl;
    // std::cout << "Optimizer Budget=" << gl->optimizer_budget() << std::endl;

    auto opt = std::make_shared< exam_optimizer >( gl );

    // Optimize F in range [1, 20] (heuristic)
    opt->optimize( optimizer_strategy::MINIMIZE, 1, 2 * gl->S );

    double J_opt = gl->get_optimizer_result();
    auto params = gl->get_optimizer_optimal_parameters();
    int F_opt = 0;
    if ( !params.empty() )
        F_opt = params[0];

    // Back-calculate R
    // J = 10*F + 2*R  =>  2*R = J - 10*F  => R = (J - 10*F) / 2
    double R_opt = ( J_opt - 10.0 * F_opt ) / 2.0;

    std::cout << "R " << R_opt << std::endl;
    std::cout << "F " << F_opt << std::endl;
    std::cout << "J " << J_opt << std::endl;

    return 0;
}
