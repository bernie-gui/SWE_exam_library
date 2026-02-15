/**
 * @file main.cpp
 * @brief Soluzione Esercizio 1 - Simulation of Software Development MDP
 * @details Implementazione di una simulazione Montecarlo per il calcolo del costo atteso
 *          in un Markov Decision Process (MDP) utilizzando la libreria SWE_exam_library.
 */

#include <cassert>
#include <memory>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <string>
#include <limits>

// Inclusione dei file header della libreria fornita
#include "common.hpp"
#include "global.hpp"
#include "process.hpp"
#include "system.hpp"
#include "simulator.hpp"
#include "random.hpp"
#include "montecarlo.hpp"
#include "io/input_parser.hpp"
#include "io/output_writer.hpp"

using namespace isw;

// ---------------------------------------------------------------------------
// Strutture Dati
// ---------------------------------------------------------------------------

/**
 * @brief Rappresenta una transizione nel MDP.
 */
struct transition_t {
    size_t target;  // Nodo di destinazione
    double prob;    // Probabilità di transizione (P)
    double cost;    // Costo della transizione (C)
};

// ---------------------------------------------------------------------------
// Classi Personalizzate
// ---------------------------------------------------------------------------

/**
 * @brief Classe globale personalizzata per mantenere lo stato del MDP.
 */
class mdp_global_t : public global_t
{
public:
    mdp_global_t() : global_t() {}

    // Parametri del MDP
    size_t num_states = 0;
    std::vector<std::vector<transition_t>> adj_list; // Lista di adiacenza del grafo

    // Stato della singola simulazione
    double accumulated_cost = 0.0;
    bool reached_terminal = false;

    /**
     * @brief Inizializza le variabili per una nuova run Montecarlo.
     */
    void init() override {
        global_t::init();
        accumulated_cost = 0.0;
        reached_terminal = false;
        // Resetta il valore corrente per il Montecarlo
        set_montecarlo_current(0.0);
    }
};

/**
 * @brief Thread che simula il "walker" nel grafo MDP.
 */
class mdp_thread_t : public thread_t
{
private:
    size_t current_node;

public:
    mdp_thread_t() : thread_t(0.0, 0.0, 0.0), current_node(0) {}

    /**
     * @brief Inizializza il thread posizionando il walker nello stato 0.
     */
    void init() override {
        thread_t::init();
        current_node = 0;
    }

    /**
     * @brief Esegue un passo della simulazione (transizione di stato).
     */
    void fun() override {
        auto global = get_global<mdp_global_t>();

        // Se abbiamo raggiunto lo stato finale (N-1), la simulazione per questo run è tecnicamente finita
        if (current_node == global->num_states - 1) {
            global->reached_terminal = true;
            // Assicuriamoci che il valore finale sia registrato
            global->set_montecarlo_current(global->accumulated_cost);
            return; 
        }

        // Recupera le transizioni possibili dallo stato corrente
        const auto& transitions = global->adj_list[current_node];
        
        // Se non ci sono transizioni (non dovrebbe accadere tranne che nel finale), ci fermiamo
        if (transitions.empty()) {
            return;
        }

        // Selezione randomica della prossima transizione basata sulla probabilità
        double r = global->get_random()->uniform_range(0.0, 1.0);
        double cumulative_prob = 0.0;
        
        for (const auto& trans : transitions) {
            cumulative_prob += trans.prob;
            if (r <= cumulative_prob) {
                // Esegui transizione
                global->accumulated_cost += trans.cost;
                current_node = trans.target;
                
                // Aggiorna il valore corrente per il Montecarlo
                global->set_montecarlo_current(global->accumulated_cost);
                break;
            }
        }

        // Avanza leggermente il tempo per permettere allo scheduler di procedere
        // (Anche se il tempo fisico non influenza il costo in questo modello)
        set_thread_time(get_thread_time() + 0.1);
    }
};

/**
 * @brief Simulatore personalizzato per gestire la condizione di terminazione.
 */
class mdp_simulator_t : public simulator_t
{
public:
    using simulator_t::simulator_t;

    /**
     * @brief Verifica se la simulazione corrente deve terminare.
     * @return true se lo stato finale è stato raggiunto.
     */
    virtual bool should_terminate() override
    {
        auto global = get_global<mdp_global_t>();
        // Termina se il walker ha raggiunto lo stato N-1
        return global->reached_terminal;
    };
};

/**
 * @brief Parser per leggere il file parameters.txt.
 */
class mdp_input_parser_t : public input_parser_t
{
private:
    std::shared_ptr<mdp_global_t> _global;

public:
    mdp_input_parser_t(const std::string& filename, std::shared_ptr<mdp_global_t> global) 
        : input_parser_t(filename), _global(global) {}

    void parse() override {
        auto& iss = get_stream();
        std::string token;

        while (iss >> token) {
            if (token == "N") {
                size_t n;
                iss >> n;
                _global->num_states = n;
                _global->adj_list.resize(n);
            } 
            else if (token == "A") {
                size_t src, dst;
                double prob, cost;
                iss >> src >> dst >> prob >> cost;

                if (src < _global->num_states) {
                    transition_t t;
                    t.target = dst;
                    t.prob = prob;
                    t.cost = cost;
                    _global->adj_list[src].push_back(t);
                }
            }
        }
    }
};

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------

int main() {
    try {
        // 1. Inizializzazione Global
        auto global = std::make_shared<mdp_global_t>();
        
        // 2. Parsing dei parametri
        // Lettura del file parameters.txt per costruire il grafo MDP
        mdp_input_parser_t parser("parameters.txt", global);
        parser.parse();

        // Configurazione Budget Montecarlo come richiesto (1000 simulazioni)
        global->set_montecarlo_budget(1000);

        // 3. Creazione del Sistema
        auto system = system_t::create(global, "mdp_system");
        
        // Creazione del Processo e del Thread
        auto process = process_t::create("walker_process");
        process->add_thread(std::make_shared<mdp_thread_t>());
        system->add_process(process);

        // 4. Configurazione Simulazione
        auto simulator = std::make_shared<mdp_simulator_t>(system);
        auto montecarlo = montecarlo_t::create(simulator);

        // 5. Esecuzione Montecarlo
        // Questo eseguirà 1000 run, calcolando la media dei costi
        montecarlo->run();

        // 6. Scrittura Output
        auto writer = output_writer_t("results.txt");
        
        // Prima riga formattata come richiesto
        writer.write_line("2025-01-09-AntonioMario-RossiPatrizio-1234567");
        
        // Seconda riga con il costo atteso (media Montecarlo)
        // Usiamo un buffer stringa per formattare l'output
        writer << "C " << global->get_montecarlo_avg() << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Errore durante l'esecuzione: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}