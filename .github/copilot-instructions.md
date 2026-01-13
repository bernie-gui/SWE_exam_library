# Copilot Instructions for SWE_exam_library

This is a C++17 Discrete Event Simulation (DES) library for the "Ingegneria Software" exam at Sapienza University.

## Architecture & Core Components

- **Namespace**: All code resides in `isw`. Use `using namespace isw;` in examples.
- **System Hierarchy**: `system_t` -> `process_t` -> `thread_t`.
  - `system_t`: Top-level container. Creates processes.
  - `process_t`: Represents a node/actor. Creates threads.
  - `thread_t`: The active unit implementation logic. Override `fun()`.
  - `simulator_t`: Manages the simulation loop. Override `should_terminate()`.
- **Global State**:
  - Inherit from `global_t` to store scenario-specific data (e.g., config, matrices, shared vars).
  - Access in threads via `get_global<MyGlobalType>()`.
- **Factory Pattern**:
  - Use static `create()` methods returning `std::shared_ptr` for `system_t`, `process_t`, `montecarlo_t`, `logger_t`.
  - Use `std::make_shared<T>()` for custom implementations of `thread_t`, `simulator_t`, `global_t`.

## Developer Workflows

- **Build Support**:
  - `make` : Builds the main target in debug mode (default).
  - `make examples` : Compiles all examples in `examples/`.
  - `make BUILD=release` : Optimized build.
- **Running**:
  - Binaries are output to the root directory (e.g., `./example-mdp_1`).
  - Examples serve as the primary integration tests.

## Coding Conventions

- **Input/Output**:
  - Inherit `input_parser_t` for config loading.
  - Use `logger_t` for CSV output. Pattern: `create` -> `add_field`... -> `log_fields` -> loop[`add_measurement`... -> `log_measurement`].
- **Network**:
  - Use `network::message_t` for communication.
  - Sending: `send_message("dest_process_name", port, msg)`.
  - Receiving: `auto msg = receive_message()`.
- **Randomness**: 
  - Access via `global->get_random()`.
  - Methods: `uniform_range`, `gaussian_sample`, etc.

## Example Usage Pattern

```cpp
// 1. Define Global State
class my_global : public global_t { ... };

// 2. Define Thread Logic
class my_thread : public thread_t {
    void fun() override {
        auto global = get_global<my_global>();
        // logic with global->get_random()...
        set_sleep_time(...); 
    }
};

// 3. Main Setup
auto global = std::make_shared<my_global>();
auto system = system_t::create(global, "sys_name");
auto proc = process_t::create("proc_name");
proc->add_thread(std::make_shared<my_thread>());
system->add_process(proc);
auto sim = std::make_shared<my_simulator>(system);
sim->run();
```
