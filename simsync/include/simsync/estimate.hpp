#ifndef SIMSYNC_SIMULATE_HPP
#define SIMSYNC_SIMULATE_HPP

#include <chrono>
#include <deque>
#include <memory>

/**
 * A library for modelling multi-threaded applications.
 */
namespace simsync {
class application;
class system;
class report;

/**
 * Simulate how an simsync::application would run on a simsync::system.
 *
 * @param app The application to run.
 * @param sys The system to run the application on.
 * @param out The reports to generate during this simulation.
 *
 * @return an estimate of the application's execution time.
 */
std::chrono::nanoseconds
estimate(application const &app, system &sys, std::deque<std::unique_ptr<report>> const &reports);
}

#endif //SIMSYNC_SIMULATE_HPP
