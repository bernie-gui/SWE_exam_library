#include "utils/rate.hpp"

isw::utils::rate_meas_t::rate_meas_t(): _rate(0), _last_time(0) {}

void isw::utils::rate_meas_t::update(double amount, double time) {
    _rate = _rate * ( _last_time / time ) + amount / time;
    _last_time = time;
}

void isw::utils::rate_meas_t::init() {
    _rate = _last_time = 0;
}

double isw::utils::rate_meas_t::get_rate() {return _rate;}