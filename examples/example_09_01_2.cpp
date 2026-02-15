/**
 * @file main.cpp
 * @brief Soluzione Esercizio 2 - Stima Probabilità Costo MDP
 * @details Implementazione di una simulazione Montecarlo per stimare la probabilità
 *          che il costo totale di un Markov Decision Process (MDP) sia <= C.
 *          Utilizza la libreria SWE_exam_library.
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
 * @brief Classe globale personalizzata per mantenere lo stato del MDP e i parametri.
 */
class mdp_global_t : public global_t
{
public:
    mdp_global_t() : global_t() {}

    // Parametri statici del MDP (caricati da file)
    size_t num_states = 0;                          // N: Numero di stati
    double max_allowed_cost = 0.0;                  // C: Costo massimo ammissibile (soglia)
    std::vector<std::vector<transition_t>> adj_list; // Grafo delle transizioni

    // Stato dinamico della singola simulazione
    double accumulated_cost = 0.0;                  // Costo accumulato nella run corrente
    bool reached_terminal = false;                  // Flag indicante se lo stato finale è raggiunto

    /**
     * @brief Inizializza le variabili per una nuova run Montecarlo.
     * @details Resetta il costo accumulato e il flag di terminazione.
     */
    void init() override {
        global_t::init();
        accumulated_cost = 0.0;
        reached_terminal = false;
        set_montecarlo_current(0.0); // Default a fallimento se non termina
    }
};

/**
 * @brief Thread che simula l'attraversamento del grafo MDP.
 */
class mdp_thread_t : public thread_t
{
private:
    size_t current_node;

public:
    mdp_thread_t() : thread_t(0.0, 0.0, 0.0), current_node(0) {}

    /**
     * @brief Inizializza il thread posizionando il cursore sullo stato iniziale (0).
     */
    void init() override {
        thread_t::init();
        current_node = 0;
    }

    /**
     * @brief Esegue un passo della simulazione.
     * @details Sceglie probabilisticamente la prossima transizione, accumula il costo
     *          e verifica la condizione di successo alla terminazione.
     */
    void fun() override {
        auto global = get_global<mdp_global_t>();

        // Verifica se siamo arrivati allo stato terminale (N-1)
        if (current_node == global->num_states - 1) {
            global->reached_terminal = true;
            
            // Calcolo risultato Montecarlo per l'Esercizio 2:
            // 1.0 se il costo <= C (Successo), 0.0 altrimenti (Fallimento).
            // La media di questi valori su 1000 run darà la Probabilità.
            double result = (global->accumulated_cost <= global->max_allowed_cost) ? 1.0 : 0.0;
            global->set_montecarlo_current(result);
            return; 
        }

        // Recupera le transizioni uscenti dal nodo corrente
        const auto& transitions = global->adj_list[current_node];
        
        // Se non ci sono transizioni (deadlock), termina (non dovrebbe accadere per specifica)
        if (transitions.empty()) {
            return;
        }

        // Selezione randomica della prossima transizione
        double r = global->get_random()->uniform_range(0.0, 1.0);
        double cumulative_prob = 0.0;
        
        for (const auto& trans : transitions) {
            cumulative_prob += trans.prob;
            if (r <= cumulative_prob) {
                // Applica la transizione
                global->accumulated_cost += trans.cost;
                current_node = trans.target;
                break;
            }
        }

        // Avanzamento fittizio del tempo per permettere allo scheduler di ciclare
        set_thread_time(get_thread_time() + 0.1);
    }
};

/**
 * @brief Simulatore per gestire la condizione di stop.
 */
class mdp_simulator_t : public simulator_t
{
public:
    using simulator_t::simulator_t;

    /**
     * @brief Determina se la simulazione deve fermarsi.
     * @return true se lo stato finale è stato raggiunto.
     */
    virtual bool should_terminate() override
    {
        auto global = get_global<mdp_global_t>();
        return global->reached_terminal;
    };
};

/**
 * @brief Parser per il file parameters.txt specifico dell'Esercizio 2.
 * @details Gestisce il formato:
 *          C <costo_max>
 *          N <stati>
 *          A i j P Cost
 */
class mdp_input_parser_t : public input_parser_t
{
private:
    std::shared_ptr<mdp_global_t> _global;

public:
    mdp_input_parser_t(const std::string& filename, std::shared_ptr<mdp_global_t> global) 
        : input_parser_t(filename), _global(global) {}

    /**
     * @brief Esegue il parsing del file.
     */
    void parse() override {
        auto& iss = get_stream();
        std::string token;

        // Loop di lettura basato sui token
        while (iss >> token) {
            if (token == "C") {
                // Lettura costo massimo ammissibile (nuovo parametro Ex 2)
                double cost_threshold;
                iss >> cost_threshold;
                _global->max_allowed_cost = cost_threshold;
            }
            else if (token == "N") {
                // Lettura numero stati e ridimensionamento grafo
                size_t n;
                iss >> n;
                _global->num_states = n;
                _global->adj_list.resize(n);
            } 
            else if (token == "A") {
                // Lettura arco: Sorgente, Destinazione, Probabilità, Costo
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
        // Lettura configurazione MDP e soglia C
        mdp_input_parser_t parser("parameters.txt", global);
        parser.parse();

        // Configurazione Budget Montecarlo (1000 simulazioni come da specifica)
        global->set_montecarlo_budget(1000);

        // 3. Creazione del Sistema e Processi
        auto system = system_t::create(global, "mdp_prob_system");
        
        auto process = process_t::create("walker_process");
        process->add_thread(std::make_shared<mdp_thread_t>());
        system->add_process(process);

        // 4. Configurazione ed Esecuzione Simulazione
        auto simulator = std::make_shared<mdp_simulator_t>(system);
        auto montecarlo = montecarlo_t::create(simulator);

        // Esegue le 1000 run.
        // Ad ogni run, il thread imposta il valore Montecarlo a 1 (successo) o 0 (fallimento).
        // Alla fine, global->get_montecarlo_avg() conterrà la probabilità stimata.
        montecarlo->run();

        // 5. Scrittura Output
        auto writer = output_writer_t("results.txt");
        
        // Prima riga standard
        writer.write_line("2025-01-09-AntonioMario-RossiPatrizio-1234567");
        
        // Seconda riga: P <valore probabilità>
        writer << "P " << global->get_montecarlo_avg() << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Errore irreversibile: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}