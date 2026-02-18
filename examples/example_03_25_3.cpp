#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <memory>
// #include <tuple>
#include <ostream>
#include <vector>
// #include "utils/customer-server/utils.hpp"
// #include "utils/customer-server/server.hpp"
#include "global.hpp"
#include "io/array_parser.hpp"
#include "montecarlo.hpp"
#include "network/message.hpp"
#include "process.hpp"
#include "simulator.hpp"
#include "system.hpp"
#include "utils/customer-server/server.hpp"
using namespace isw;

const int TIMESTEP = 1;

class requests_global : public global_t
{
public:
    size_t n, k, oversell;
    double a;
    std::vector< double > p, f;
    void init() override
    {
        global_t::init();
        oversell = 0;
        // measure_v.init();
        // measure_w.init();
    }
};


class my_msg : public network::message_t
{
public:
    size_t item;
    bool success;
};


class cust_thread_1 : public thread_t
{
public:
    void fun() override
    {
        // size_t server;
        // cs::request_t msg;
        auto gl = get_global< requests_global >();
        my_msg send;
        double prob = gl->get_random()->uniform_range( 0.0, 1.0 );
        double accum = 0;
        for ( size_t i = 0; i < gl->k; i++ )
        {
            accum += gl->p[i];
            if ( prob <= accum )
            {
                send.item = i;
                break;
            }
        }
        send_message( "Servers", get_process()->get_relative_id().value(), send );
    }
    void init() override
    {
        thread_t::init();
        auto gl = get_global< requests_global >();
        set_compute_time( TIMESTEP );
    }
};

class supplier : public thread_t
{
public:
    std::vector< size_t > *data;

    void fun() override
    {
        auto gl = get_global< requests_global >();
        double prob = gl->get_random()->uniform_range( 0.0, 1.0 );
        double accum = 0;
        for ( size_t i = 0; i < gl->k; i++ )
        {
            accum += gl->f[i];
            if ( prob <= accum )
            {
                ( *data )[i]++;
                break;
            }
        }
    }

    supplier( std::vector< size_t > *data ) : thread_t( TIMESTEP ), data( data ) {}
};

// class my_sys : public system_t {
//     public:
//         void on_end_step() override {
//             auto &ps = get_processes();
//             // for (auto p : ps) {
//             //     std::cout << p->get_world_key().value() << " - "
//             //         << p->get_relative_id().value() << ": " << p->next_update_time() << std::endl;
//             // }
//         }

//         my_sys(std::shared_ptr<requests_global> gl) : system_t(gl) {}
// };

class sim_help : public simulator_t
{
public:
    void on_terminate() override
    {
        static int iter = 0;
        std::cout << "Montecarlo iteration: " << iter++ << std::endl;
        auto gl = get_global< requests_global >();
        // gl->measure.update(0, gl->get_horizon()); //tecnicamente corretto ma cambia poco
        gl->set_montecarlo_current( gl->oversell );
        // gl->set_montecarlo_current(gl->measure_w.get_rate(), 1);
    }
    sim_help( std::shared_ptr< system_t > s ) : simulator_t( s ) {}
};

int main()
{
    auto gl = std::make_shared< requests_global >();
    auto sys = std::make_shared< system_t >( gl );
    array_parser_t reader( "examples/example_03_25_3.txt",
                           {
                               [gl]( auto &iss )
                               {
                                   double temp;
                                   iss >> temp;
                                   gl->set_horizon( temp );
                               },
                               [gl]( auto &iss ) { iss >> gl->n; },
                               [gl]( auto &iss )
                               {
                                   iss >> gl->k;
                                   gl->p.resize( gl->k );
                                   gl->f.resize( gl->k );
                               },
                               [gl]( auto &iss ) { iss >> gl->a; },
                               [gl]( auto &iss )
                               {
                                   for ( size_t i = 0; i < gl->k; i++ )
                                       iss >> gl->p[i];
                               },
                               [gl]( auto &iss )
                               {
                                   for ( size_t i = 0; i < gl->k; i++ )
                                       iss >> gl->f[i];
                               },
                           } );
    reader.parse();
    std::vector< size_t > database( gl->k );
    std::fill( database.begin(), database.end(), 10 );
    for ( size_t i = 0; i < gl->n; i++ )
    {
        sys->add_process( cs::server_t::create_process< my_msg >(
                              gl->k * 2, [gl, &database]( auto i ) { return i % 2 == 0 ? 0 : database[i / 2]; },
                              TIMESTEP,
                              { { "Customers",
                                  [gl, &database]( auto pt, auto msg )
                                  {
                                      auto p = pt->template get_process< cs::server_t >();
                                      my_msg ret;
                                      double extract;
                                      // std::cout << "ok" << std::endl;
                                      // std::cout << msg->item << std::endl;
                                      if ( p->database[msg->item * 2] - p->database[msg->item * 2 + 1] == 0 )
                                          ret.success = false;
                                      else
                                      {
                                          ret.success = true;
                                          p->database[msg->item * 2 + 1]++;
                                          extract = gl->get_random()->uniform_range( 0.0, 1.0 );
                                          if ( extract <= gl->a )
                                          {
                                              pt->set_thread_time( pt->get_thread_time() + 9 ); // funzione di libreria
                                              for ( size_t i = 0; i < gl->k; i++ )
                                              {
                                                  if ( database[i] < p->database[i * 2 + 1] )
                                                  {
                                                      database[i] = 0;
                                                      gl->oversell++;
                                                  }
                                                  else
                                                      database[i] -= p->database[i * 2 + 1];
                                                  p->database[i * 2 + 1] = 0;
                                                  p->database[i * 2] = database[i];
                                              }
                                          }
                                      }
                                      // std::cout << "ok2" << std::endl;
                                      ret.item = msg->item;
                                      // pt->send_message("Customers", p->get_relative_id().value(), ret);
                                  } } } ),
                          "Servers" );
        sys->add_process( std::make_shared< process_t >()->add_thread( std::make_shared< cust_thread_1 >() ),
                          "Customers" );
    }
    sys->add_process( std::make_shared< process_t >()->add_thread( std::make_shared< supplier >( &database ) ) );
    sys->add_network();

    auto sim = std::make_shared< sim_help >( sys );
    // sim->run();

    auto monty = montecarlo_t::create( sim );
    gl->set_montecarlo_budget( 50 );
    monty->run();

    std::cout << "S " << gl->get_montecarlo_avg() << std::endl
              << "R " << gl->get_montecarlo_avg() / ( gl->n * gl->get_horizon() ) << std::endl;
    return 0;
}
