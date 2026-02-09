#pragma once

// TODO: documentation
namespace isw::utils {

    class rate_meas_t {
        public:
            rate_meas_t();

            void update(double amount, double time);

            double get_rate();

            void init();

        private:
            double _rate, _last_time;
    };
}