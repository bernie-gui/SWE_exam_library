#include <cassert>
#include <memory>
#include <stdexcept>
#include <cmath>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iostream>

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

// Struttura per memorizzare lo stato di un veicolo ad un dato istante
struct vehicle_record_t {
    double t;
    size_t id;
    double x;
    double y;
};

class my_global_t : public global_t
{
public:
	my_global_t() : global_t() {}

    // Parametri di simulazione
    double horizon = 0.0;
    size_t n_vehicles = 0;
    double velocity = 0.0;
    double time_step = 0.0;

    // Buffer per memorizzare i risultati da scrivere alla fine
    std::vector<vehicle_record_t> records;
};

class my_input_parser_t : public input_parser_t
{
public:
    my_input_parser_t( std::shared_ptr< global_t > global ) :
        input_parser_t( global, "parameters.txt" )
    {
    }

    void parse() override
    {
        auto global = get_global< my_global_t >();
        std::ifstream& in = get_stream();
        std::string key;
        
        // Parsing dei parametri dal file
        while (in >> key) {
            if (key == "H") {
                in >> global->horizon;
            } else if (key == "N") {
                in >> global->n_vehicles;
            } else if (key == "V") {
                in >> global->velocity;
            } else if (key == "T") {
                in >> global->time_step;
            }
        }
    }
};

class my_thread_t : public thread_t
{
public:
    // Inizializza il thread per partire al tempo 0
	my_thread_t() : thread_t( 0.0, 0.0, 0.0 ), x(0.0), y(0.0) {}

    double x;
    double y;

    void init() override {
        // Inizializzazione posizione random in [-10, 10]
        auto global = get_global< my_global_t >();
        auto rng = global->get_random();
        
        x = rng->uniform_range(-10.0, 10.0);
        y = rng->uniform_range(-10.0, 10.0);
        
        // Reset dei tempi per assicurare partenza a t=0
        set_thread_time(0.0);
        set_compute_time(0.0);
        set_sleep_time(0.0);
    }

	void fun() override {
        auto global = get_global< my_global_t >();
        auto process = get_process();
        double current_time = process->get_system()->get_current_time();
        
        // Recupera ID del processo (veicolo), assume 0-based dal sistema
        size_t id = process->get_id().value_or(0);

        // 1. Registra la posizione corrente (prima del movimento)
        // L'ID nel file di output deve essere 1-based (i=1...N)
        global->records.push_back({current_time, id + 1, x, y});

        // 2. Calcola il movimento per il prossimo step
        // Sceglie una direzione random theta in [0, 2pi]
        double theta = global->get_random()->uniform_range(0.0, 2.0 * 3.14159265358979323846);
        
        // Aggiorna posizione: pos(t+T) = pos(t) + T * V * dir
        double dist = global->time_step * global->velocity;
        x += dist * std::sin(theta);
        y += dist * std::cos(theta);

        // 3. Imposta sleep per svegliarsi al prossimo step T
        set_compute_time(0.0); // Movimento istantaneo logico
        set_sleep_time(global->time_step);
	}
};

class my_simulator_t : public simulator_t
{
public:
    using simulator_t::simulator_t;

    virtual bool should_terminate()
    {
        auto global = get_global< my_global_t >();
        // Termina se il tempo corrente supera l'orizzonte
        return get_system()->get_current_time() > global->horizon;
    };
};

int main() {
	auto global = std::make_shared< my_global_t >();
    
    // Scrittura intestazione output
	auto writer =  output_writer_t( "results.txt" );
    writer.write_line("2025-01-09-AntonioMario-RossiPatrizio-1234567");

    // Parsing parametri
    my_input_parser_t parser( global );
    parser.parse();

    // Creazione sistema
    auto system = system_t::create( global, "uv_system" );

    // Creazione veicoli (Processi + Thread)
    for(size_t i = 0; i < global->n_vehicles; ++i) {
        auto process = process_t::create("vehicle_" + std::to_string(i));
        process->add_thread( std::make_shared< my_thread_t >() );
        system->add_process( process );
    }

    // Esecuzione simulazione
    auto simulator = std::make_shared< my_simulator_t >( system );
    simulator->run();

    // Ordinamento dei risultati: per tempo crescente, poi per ID veicolo crescente
    std::sort(global->records.begin(), global->records.end(), 
        [](const vehicle_record_t& a, const vehicle_record_t& b) {
            if (std::abs(a.t - b.t) > 1e-9) return a.t < b.t;
            return a.id < b.id;
        }
    );

    // Scrittura dei risultati su file
    for(const auto& rec : global->records) {
        std::stringstream ss;
        ss << rec.t << " " << rec.id << " " << rec.x << " " << rec.y;
        writer.write_line(ss.str());
    }

    return 0;
}